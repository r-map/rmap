#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <frtosLog.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <esp_rom_crc.h>
#include "TimeLib.h"

#define SOFTWARE_VERSION "1.0"

/*!
\def DISABLE_LOGGING
\brief fully disable logging
define the macro to fully disable logging, and reduce project size
*/
//#define DISABLE_LOGGING

/*!
\def LOG_LEVEL
\brief logging level at compile time
Available levels are:
LOG_LEVEL_SILENT
LOG_LEVEL_FATAL
LOG_LEVEL_ERROR
LOG_LEVEL_WARNING
LOG_LEVEL_NOTICE
LOG_LEVEL_TRACE
LOG_LEVEL_VERBOSE
*/
#define LOG_LEVEL   LOG_LEVEL_NOTICE
#define RESET_PAIR false

// Definisci la PMK globale (esattamente 16 byte)
// Deve essere IDENTICA su tutti i dispositivi che comunicano tra loro.
const uint8_t mia_pmk[16] = {
    0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF6, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x11
};

// Definisci la chiave LMK (esattamente 16 byte)
// Puoi usare valori esadecimali a tua scelta. Entrambi i dispositivi devono avere la stessa chiave.
const uint8_t mio_lmk[16] = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10
};


// REPLACE WITH THE MAC Address of your receiver 
//uint8_t broadcastAddress[] = {0x18,0x8b,0x0e,0x04,0x2c,0x0c};
//uint8_t broadcastAddress[] = {0x70,0x04,0x1d,0x22,0x73,0xe8};
uint8_t broadcastAddress[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

struct struct_config {
  bool accoppiato = false;
  uint8_t satelliteMac[6];
  uint8_t channel = 3;
};
struct_config config;

MutexStandard loggingmutex;

//Structure to send pair messages
//Must match the receiver structure
struct message_pair {
  time_t datetime;
};

struct message_pair_crc {
  int type;
  message_pair message;
  uint8_t crc;
};

//Structure to send data
//Must match the receiver structure
struct message_data {
  time_t datetime;
  float temp;
  float hum;
  float pres;
};

struct message_data_crc {
  int type;
  message_data message;
  uint8_t crc;
};

// prefix for logging system
void logPrefix(Print* _logOutput) {
#define DATE_TIME_STRING_LENGTH                       (25)
  char dt[DATE_TIME_STRING_LENGTH];
  snprintf(dt, DATE_TIME_STRING_LENGTH, "%04u-%02u-%02uT%02u:%02u:%02u", year(), month(), day(), hour(), minute(), second());
  _logOutput->print("#");
  _logOutput->print(dt);
  _logOutput->print(" ");
}

// suffix for logging system
void logSuffix(Print* _logOutput) {
  _logOutput->print('\n');
  _logOutput->flush();  // we use this to flush every log message
}

// read configuration from EEPROM
bool read_local_config() {

  if (LittleFS.exists("/master.json")) {
    //file exists, read and load
    frtosLog.notice(F("reading rmap config file"));
    File configFile = LittleFS.open("/master.json", "r");
    if (configFile) {
      frtosLog.notice(F("opened master config file"));
      
      //size_t size = configFile.size();
      // Allocate a buffer to store contents of the file.
      //std::unique_ptr<char[]> buf(new char[size]);
      //configfile.readBytes(buf.get(), size);
      String content = configFile.readString();
      configFile.close();
      
      DynamicJsonDocument doc(200);
      DeserializationError error = deserializeJson(doc,content);
      if (!error){
	const char* ver = doc["ver"]; // "1.0"
	config.accoppiato = doc["accoppiato"];
	JsonArray satellitemac = doc["satellitemac"];
	config.satelliteMac[0]= satellitemac[0]; // 1
	config.satelliteMac[1]= satellitemac[1]; // 2
	config.satelliteMac[2]= satellitemac[2]; // 3
	config.satelliteMac[3]= satellitemac[3]; // 4
	config.satelliteMac[4]= satellitemac[4]; // 5
	config.satelliteMac[5]= satellitemac[5]; // 6
	config.channel = doc["channel"];
	frtosLog.notice(F("accoppiato: %T"),config.accoppiato);
	frtosLog.notice(F("MAC 0: %X"),config.satelliteMac[0]);
	frtosLog.notice(F("MAC 1: %X"),config.satelliteMac[1]);
	frtosLog.notice(F("MAC 2: %X"),config.satelliteMac[2]);
	frtosLog.notice(F("MAC 3: %X"),config.satelliteMac[3]);
	frtosLog.notice(F("MAC 4: %X"),config.satelliteMac[4]);
	frtosLog.notice(F("MAC 5: %X"),config.satelliteMac[5]);
	frtosLog.notice(F("channel %d"),config.channel);
	
	return true;
      } else {
	frtosLog.error(F("reading master file: %s"),error.c_str());	
      }
    } else {
      frtosLog.warning(F("master file do not exist"));
    }
  }
  return false;
}

// write configuration to EEPROM
bool write_local_config() {

  //save the custom parameters to FS
  frtosLog.notice(F("saving master config"));
  
  File configFile = LittleFS.open("/master.json", "w");
  if (!configFile) {
    frtosLog.error(F("failed to open rmap config file for writing"));
    return false;
  }

  DynamicJsonDocument doc(200); 
  doc["ver"] = SOFTWARE_VERSION;
  doc["accoppiato"] = config.accoppiato;
  doc["satellitemac"][0] = config.satelliteMac[0];
  doc["satellitemac"][1] = config.satelliteMac[1];
  doc["satellitemac"][2] = config.satelliteMac[2];
  doc["satellitemac"][3] = config.satelliteMac[3];
  doc["satellitemac"][4] = config.satelliteMac[4];
  doc["satellitemac"][5] = config.satelliteMac[5];

  /*
  JsonArray satellitemac = doc["satellitemac"].to<JsonArray>();
  satellitemac.add(config.satelliteMac[0]);
  satellitemac.add(config.satelliteMac[1]);
  satellitemac.add(config.satelliteMac[2]);
  satellitemac.add(config.satelliteMac[3]);
  satellitemac.add(config.satelliteMac[4]);
  satellitemac.add(config.satelliteMac[5]);
  */
  
  doc["channel"] = config.channel;
  char buffer[256];
  serializeJson(doc, buffer, sizeof(buffer));
  configFile.print(buffer);
  configFile.close();
  frtosLog.notice(F("saved master config parameter"));
  //end save
  return true;
}

void readMacAddress(){
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    frtosLog.notice("my MAC %X:%X:%X:%X:%X:%X",
		    baseMac[0], baseMac[1], baseMac[2],
		    baseMac[3], baseMac[4], baseMac[5]);
  } else {
    frtosLog.error("Failed to read MAC address");
  }
}

void add_broadcast_peer(){
  // Register peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  //peerInfo.ifidx = WIFI_IF_STA;  // Interfaccia usata (Station o AP)
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  if (esp_now_is_peer_exist(peerInfo.peer_addr)){
    frtosLog.notice("peer broadcast già registrato");
  }else{    
    // Add peer        
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
      frtosLog.error("Failed to add broadcast peer");
    }
  }
}


// Callback when data is sent
void OnDataSent(const  uint8_t *des_addr, esp_now_send_status_t status) {
  frtosLog.notice("Last Packet Send Status: %s", status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail"  );
  if (status != ESP_NOW_SEND_SUCCESS) frtosLog.error("Error sending");
  frtosLog.notice(F("destination MAC: %X:%X:%X:%X:%X:%X"),
		  des_addr[0], des_addr[1], des_addr[2],
		  des_addr[3], des_addr[4], des_addr[5]);
}

// Callback when data is received
void OnDataRecv(const esp_now_recv_info_t *esp_now_info, const uint8_t *incomingData, int len) {
  // Create a message_pair to hold incoming sensor readings
  frtosLog.notice(F("Pacchetto ricevuto da MAC: : %X:%X:%X:%X:%X:%X"),
		  esp_now_info->src_addr[0], esp_now_info->src_addr[1], esp_now_info->src_addr[2],
		  esp_now_info->src_addr[3], esp_now_info->src_addr[4], esp_now_info->src_addr[5]);

  frtosLog.notice(F("Bytes received: %d"),len);
  
  // Controlla se è una richiesta di Pairing
  uint16_t type;
  memcpy(&type, incomingData, sizeof(type));
  if (type == 1 ) {
    frtosLog.notice("risposta al broadcast ricevuta");
    message_pair_crc incomingMessage;
    memcpy(&incomingMessage, incomingData, len);
    uint8_t crc = esp_rom_crc8_le(0, (const uint8_t*)&incomingMessage.message,sizeof(incomingMessage.message));
    frtosLog.notice(F("computed CRC: %d"),crc);
    if (crc != incomingMessage.crc){
      frtosLog.error("CRC mismatch");
      return;
    }
    if (esp_now_is_peer_exist(esp_now_info->src_addr)){
      frtosLog.notice("peer già registrato");
    }else{    
      // Aggiunge il satellite come peer specifico
      esp_now_peer_info_t peerInfo = {};
      memcpy(peerInfo.peer_addr, esp_now_info->src_addr, 6);
      peerInfo.channel = 0;
      //peerInfo.ifidx = WIFI_IF_STA;  // Interfaccia usata (Station o AP)
      peerInfo.encrypt = false;         // enable with pioarduino only! tasmota configurated with no encryption
      memcpy(peerInfo.lmk, mio_lmk, 16);
      
      if (esp_now_add_peer(&peerInfo) == ESP_OK) {
	frtosLog.notice("Satellite registrato come peer fisso.");      
	frtosLog.notice("TU Accoppiamento riuscito!");
	memcpy(config.satelliteMac, esp_now_info->src_addr, 6); // Salva il MAC reale del satellite

	// Risponde al trasmettitore per confermare il pairing
	// Create a message_pair called Readings to hold sensor readings
	message_pair_crc outgoingMessage;
	outgoingMessage.type=2;
	outgoingMessage.message.datetime=now();
	outgoingMessage.crc = esp_rom_crc8_le(0, (const uint8_t*)&outgoingMessage.message, sizeof(outgoingMessage.message));
	esp_now_send(broadcastAddress, (uint8_t *) &outgoingMessage, sizeof(outgoingMessage));

	config.accoppiato=true;
	esp_now_del_peer(broadcastAddress);
	
	if (!write_local_config())frtosLog.error("Error writing config file");
      }else{
	frtosLog.error("Error adding peer");
      }
    }
  } else if (type == 99) {

    message_data_crc incomingMessage;
    memcpy(&incomingMessage, incomingData, len);
    uint8_t crc = esp_rom_crc8_le(0, (const uint8_t*)&incomingMessage.message,sizeof(incomingMessage.message));
    if (crc != incomingMessage.crc){
      frtosLog.error("CRC mismatch");
      return;
    }
    frtosLog.notice(F("Values received: %D %D %D"),incomingMessage.message.temp
		    ,incomingMessage.message.hum
		    ,incomingMessage.message.pres);

    message_pair_crc outgoingMessage;
    outgoingMessage.type=3;                     // data ACK
    outgoingMessage.message.datetime=now();
    outgoingMessage.crc = esp_rom_crc8_le(0, (const uint8_t*)&outgoingMessage.message, sizeof(outgoingMessage.message));
    frtosLog.notice(F("computed CRC: %d"),outgoingMessage.crc);
    esp_now_send(config.satelliteMac, (uint8_t *) &outgoingMessage, sizeof(outgoingMessage));
  }
}

void setup() {

  if (RESET_PAIR){
    frtosLog.warning(F("Reset pair information"));
    LittleFS.begin();
    LittleFS.format();
  }
  
  // Init Serial Monitor
  Serial.begin(115200);
  frtosLog.begin(LOG_LEVEL, &Serial,loggingmutex);
  frtosLog.setPrefix(logPrefix); // Uncomment to get timestamps as prefix
  frtosLog.setSuffix(logSuffix); // Uncomment to get newline as suffix

  delay(5000);
  frtosLog.notice(F("Started"));
  frtosLog.notice(F("Version: " SOFTWARE_VERSION));
  frtosLog.notice(F("Total PSRAM: %d"), ESP.getPsramSize());
    
  frtosLog.notice(F("mounting FS..."));
  if (LittleFS.begin()) {
    frtosLog.notice(F("mounted LittleFS file system"));
    if(!read_local_config()) frtosLog.error(F("failed reading config file"));
  } else {
    frtosLog.error(F("failed to mount FS"));
    frtosLog.warning(F("Reformat LittleFS"));
    LittleFS.begin();    
    LittleFS.format();
  }
  
  // Change ESP32 Mac Address
  //  esp_err_t err = esp_wifi_set_mac(WIFI_IF_STA, &newMACAddress[0]);
  //if (err == ESP_OK) {
  //  Serial.println("Success changing Mac Address");
  //}
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  frtosLog.notice("channel: %d",config.channel);
  esp_wifi_set_channel(config.channel, WIFI_SECOND_CHAN_NONE);

  // ESP-Now Range Test: Real-World Results for ESP32 Devices
  // https://youtu.be/oz0a7Ur7nko?si=aUMBJ4SpeXTSMMPg
  esp_err_t err = esp_wifi_set_protocol(WIFI_IF_STA,WIFI_PROTOCOL_LR);
  if (err == ESP_OK) {
    frtosLog.notice("Protocol successfully restricted to Long Range (LR) Mode successfully enabled!");
  } else {
    frtosLog.error("Failed to set protocol. Error code: %d", err);
  }
  
  WiFi.STA.begin();
  readMacAddress();
  
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    frtosLog.notice("Error initializing ESP-NOW");
    return;
  }

  // Imposta la PMK globale
  // Se questa funzione fallisce o non viene chiamata, ESP-NOW userà una PMK di default della mesh,
  // compromettendo la sicurezza dell'intero ecosistema.
  if (esp_now_set_pmk(mia_pmk) == ESP_OK) {
    frtosLog.notice("PMK impostata con successo!");
  } else {
    frtosLog.error("Errore nell'impostazione della PMK");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(esp_now_send_cb_t(OnDataSent));
  
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

  if (config.accoppiato){

    esp_wifi_set_channel(config.channel, WIFI_SECOND_CHAN_NONE);
    // Aggiunge il master come peer specifico
    
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, config.satelliteMac, 6);
    peerInfo.channel = 0;
    //peerInfo.ifidx = WIFI_IF_STA;  // Interfaccia usata (Station o AP)
    peerInfo.encrypt = false;         // enable with pioarduino only! tasmota configurated with no encryption
    memcpy(peerInfo.lmk, mio_lmk, 16);
    
    if (esp_now_is_peer_exist(peerInfo.peer_addr)){
      frtosLog.notice("boot peer broadcast già registrato");
    }else{    
      if (esp_now_add_peer(&peerInfo) == ESP_OK) {
	frtosLog.notice("boot Satellite registrato come peer fisso.");      
	frtosLog.notice("boot Accoppiamento riuscito!");
      }else{
	frtosLog.error("boot Error adding peer");
	config.accoppiato=false;
      }
    }
  }else{
    add_broadcast_peer();
  }

  
}
 
void loop() {
  if (!config.accoppiato){
    message_pair_crc outgoingMessage;
    outgoingMessage.type=0;
    outgoingMessage.message.datetime=now();
    outgoingMessage.crc = esp_rom_crc8_le(0, (const uint8_t*)&outgoingMessage.message, sizeof(outgoingMessage.message));
    frtosLog.notice(F("computed CRC: %d"),outgoingMessage.crc);
    frtosLog.notice("Send broadcast message");
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &outgoingMessage, sizeof(outgoingMessage));
    if (result == ESP_OK) {
      frtosLog.notice("Sent with success");
    }
    else {
      frtosLog.error("Error sending the data");
    }    
  }
  delay(1000);
}
