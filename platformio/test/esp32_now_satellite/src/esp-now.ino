// TODO gestire gli astati e quindi gli ACK


/*
idee per progettare un ipotetico sensore satellite da collegare via
radio a stimawifi dovrebbe:

* avere un raggio di azione decente
* funzionare a batterie
* avere un decente livello di sicurezza delle comunicazioni

note:

tasmota non include la crittografia, per usarla bisogna passare a
pioarduino (che occupa più memoria e ha API più nuove e con problemi)

l'uso congiunto con il wifi (non testato) implica problemi sul canale
che viene determinato (dinamicamente) dall' AP WiFi sul satellite
bisogna implementare un sistema di riceca automatica del canale

l'uso di batterie con ESP32 e quindi con risparmio energetico spinto
di fatto non permette l'uso efficiente con ricevitore radio sempre
acceso: bisogna passare alla modalità master alimentato con ricevitore
sempre acceso, saltellite attiva la comuicazione quando opportuno

questo comporta che l'invio delle misure del satellite saranno fuori
sincrono con quelle del master

se al satellite si vogliono aggiungere delle code sull'invio dei dati
riducendo la probabilità di perdita dati è necessario avere sul
satellite un timestamp ottenuto dal master

quindi logiche di funzionamento:

* ad installazione master e satellite vanno accoppiati con scambio dei MAC address
* al momento non è possibile usare crittografia, comunicazioni insecure
* il satellite fa misure e ogni tot le mette in coda per l'invio
* l'invio deve gestire un CRC con ACK ed eventuali reinvii non gestiti da esp-now
* nell'ACK del master deve essere incluso il timestamp per sincronizzare il satellite
* pensare alla gestione dell'autodiagnostica tra cui il monitoraggio delle batterie
* con il deep sleep di esp32 la ram viene persa salvo quello salvato
  nelle ram dell'RTC, è un gran casino e le code dovrebbero stare li ...
* prevedere che il master dettato dal Wifi potrebbe cambiare canale
  quando vuole e il satellite deve in autonomia provare a rifare la
  sintonia

verificare anche se l'orologio di arduino avanza in deepsleep

*/

#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <frtosLog.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <esp_rom_crc.h>
#include "TimeLib.h"

#define SOFTWARE_VERSION "1.0"
#define RESET_PAIR false

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

#define S_TO_uS_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  5          /* Time ESP32 will go to sleep (in seconds) */

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

typedef struct struct_config {
  bool accoppiato;
  uint8_t masterMac[6];
  uint8_t channel;
} struct_config;

MutexStandard loggingmutex;

//Structure to send data
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

RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR struct_config config;
RTC_DATA_ATTR uint8_t error_count=0;

// Flag per verificare se l'invio è completato
volatile bool transmissionCompleted = false;

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

/*
Method to print the reason by which ESP32
has been awaken from sleep
*/
void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:     frtosLog.notice("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1:     frtosLog.notice("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER:    frtosLog.notice("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD: frtosLog.notice("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP:      frtosLog.notice("Wakeup caused by ULP program"); break;
    default:                        frtosLog.notice("Wakeup was not caused by deep sleep: %d", wakeup_reason); break;
  }
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
	config.masterMac[0]= satellitemac[0]; // 1
	config.masterMac[1]= satellitemac[1]; // 2
	config.masterMac[2]= satellitemac[2]; // 3
	config.masterMac[3]= satellitemac[3]; // 4
	config.masterMac[4]= satellitemac[4]; // 5
	config.masterMac[5]= satellitemac[5]; // 6
	config.channel = doc["channel"];
	frtosLog.notice(F("accoppiato: %T"),config.accoppiato);
	frtosLog.notice(F("MAC 0: %X"),config.masterMac[0]);
	frtosLog.notice(F("MAC 1: %X"),config.masterMac[1]);
	frtosLog.notice(F("MAC 2: %X"),config.masterMac[2]);
	frtosLog.notice(F("MAC 3: %X"),config.masterMac[3]);
	frtosLog.notice(F("MAC 4: %X"),config.masterMac[4]);
	frtosLog.notice(F("MAC 5: %X"),config.masterMac[5]);
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
  JsonArray macArray = doc.createNestedArray("satellitemac");
  
  macArray.add(config.masterMac[0]);
  macArray.add(config.masterMac[1]);
  macArray.add(config.masterMac[2]);
  macArray.add(config.masterMac[3]);
  macArray.add(config.masterMac[4]);
  macArray.add(config.masterMac[5]);
  
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
  if (status != ESP_NOW_SEND_SUCCESS){
    error_count++;
    frtosLog.error("Error sending");
  }else{
    error_count=0;
  }
  
  frtosLog.notice(F("destination MAC: %X:%X:%X:%X:%X:%X"),
		  des_addr[0], des_addr[1], des_addr[2],
		  des_addr[3], des_addr[4], des_addr[5]);
}

// Callback when data is received
void OnDataRecv(const esp_now_recv_info_t *esp_now_info, const uint8_t *incomingData, int len) {
  // Create a struct_message to hold incoming sensor readings
  frtosLog.notice(F("Pacchetto ricevuto da MAC: : %X:%X:%X:%X:%X:%X"),
		  esp_now_info->src_addr[0], esp_now_info->src_addr[1], esp_now_info->src_addr[2],
		  esp_now_info->src_addr[3], esp_now_info->src_addr[4], esp_now_info->src_addr[5]);
  frtosLog.notice(F("Bytes received: %d"),len);
  
  // Controlla se è una richiesta di Pairing
  uint16_t type;
  memcpy(&type, incomingData, sizeof(type));
  if (type == 0) {
    frtosLog.notice("Richiesta di pairing ricevuta");
    message_pair_crc incomingMessage;
    memcpy(&incomingMessage, incomingData, len);

    uint8_t crc = esp_rom_crc8_le(0, (const uint8_t*)&incomingMessage.message, sizeof(incomingMessage.message));
    frtosLog.notice(F("computed CRC: %d"),crc);

    if (crc != incomingMessage.crc){
      frtosLog.error("CRC mismatch");
      frtosLog.error("crcc=%d  crcr=%d",crc,incomingMessage.crc);
      return;
    }

    // Risponde al trasmettitore per confermare il pairing
    // Create a struct_message called Readings to hold sensor readings
    message_pair_crc outgoingMessage;
    outgoingMessage.type=1;
    outgoingMessage.message.datetime=now();
    outgoingMessage.crc = esp_rom_crc8_le(0, (const uint8_t*)&outgoingMessage.message, sizeof(outgoingMessage.message));

    add_broadcast_peer();
    esp_now_send(broadcastAddress, (uint8_t *) &outgoingMessage, sizeof(outgoingMessage));

  } else if (type == 2 ) {
    frtosLog.notice("ACK al broadcast ricevuta");
    message_pair_crc incomingMessage;
    memcpy(&incomingMessage, incomingData, len);
    uint8_t crc = esp_rom_crc8_le(0, (const uint8_t*)&incomingMessage.message, sizeof(incomingMessage.message));
    if (crc != incomingMessage.crc){
      frtosLog.error("CRC mismatch");
      return;
    }
    
    if (esp_now_is_peer_exist(esp_now_info->src_addr)){
      frtosLog.notice("peer già registrato");
    }else{
      // Aggiunge il master come peer specifico
      esp_now_peer_info_t peerInfo = {};
      memcpy(peerInfo.peer_addr, esp_now_info->src_addr, 6);
      peerInfo.channel = 0;
      //peerInfo.ifidx = WIFI_IF_STA;  // Interfaccia usata (Station o AP)
      peerInfo.encrypt = false;        // enable with pioarduino only! tasmota configurated with no encryption
      memcpy(peerInfo.lmk, mio_lmk, 16);
      
      if (esp_now_add_peer(&peerInfo) == ESP_OK) {
	frtosLog.notice("Master registrato come peer fisso.");      
	frtosLog.notice("Accoppiamento riuscito!");
	memcpy(config.masterMac, esp_now_info->src_addr, 6); // Salva il MAC reale del master
	config.accoppiato=true;
	error_count=0;
	//Rimuove il peer broadcast generico
	esp_now_del_peer(broadcastAddress);
	if (!write_local_config())frtosLog.error("Error writing config file");
      }else{
	frtosLog.notice("Error adding peer");
      }
    }
  } else if (type == 3 ) {
    frtosLog.notice("ACK ai dati ricevuta");
    message_pair_crc incomingMessage;
    memcpy(&incomingMessage, incomingData, len);
    uint8_t crc = esp_rom_crc8_le(0, (const uint8_t*)&incomingMessage.message, sizeof(incomingMessage.message));
    if (crc != incomingMessage.crc){
      frtosLog.error("CRC mismatch");
      return;
    }
    // Sblocca il ciclo principale consentendo il deep sleep
    transmissionCompleted = true;    
  }
}

void setup() {

  // Init Serial Monitor
  Serial.begin(115200);
  frtosLog.begin(LOG_LEVEL, &Serial,loggingmutex);
  frtosLog.setPrefix(logPrefix); // Uncomment to get timestamps as prefix
  frtosLog.setSuffix(logSuffix); // Uncomment to get newline as suffix

  if (bootCount == 0 ){
    delay(5000);
    frtosLog.notice(F("Started"));
    frtosLog.notice(F("Version: " SOFTWARE_VERSION));
    frtosLog.notice(F("Total PSRAM: %d"), ESP.getPsramSize());
  }
  
  //Print the wakeup reason for ESP32
  print_wakeup_reason();

  // Change ESP32 Mac Address
  //  esp_err_t err = esp_wifi_set_mac(WIFI_IF_STA, &newMACAddress[0]);
  //if (err == ESP_OK) {
  //  frtosLog.notice("Success changing Mac Address");
  //}

  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  
  // ESP-Now Range Test: Real-World Results for ESP32 Devices
  // https://youtu.be/oz0a7Ur7nko?si=aUMBJ4SpeXTSMMPg
  esp_err_t err = esp_wifi_set_protocol(WIFI_IF_STA,WIFI_PROTOCOL_LR);
  if (err == ESP_OK) {
    frtosLog.notice("Protocol successfully restricted to Long Range (LR) Mode successfully enabled!");
  } else {
    frtosLog.notice("Failed to set protocol. Error code: %d", err);
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
    frtosLog.notice("Errore nell'impostazione della PMK");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(esp_now_send_cb_t(OnDataSent));
  
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

  
/*
 // CONFIGURAZIONE LIGHT SLEEP AUTOMATICO (Power Saving)
  
  // Forza il risparmio energetico del modem Wi-Fi in modalità MIN_MODEM
  // Questo permette all'ESP32 di alternare Light Sleep e attivazione radio autonomamente
  esp_wifi_set_ps(WIFI_PS_MIN_MODEM);

  // Imposta la "Finestra di risveglio" per ascoltare ESP-NOW (es. 30 millisecondi)
  // Durante questa finestra la radio è accesa per intercettare i dati
  uint32_t rx_wake_window = 30; 
  esp_now_set_wake_window(rx_wake_window);

  // Imposta l'intervallo di risveglio totale del modulo connectionless (es. 200 millisecondi)
  // L'ESP32 dormirà in Light Sleep per il resto del tempo (200ms - 30ms = 170ms di sonno)
  uint32_t wake_interval = 200;
  esp_wifi_connectionless_module_set_wake_interval(wake_interval);

  frtosLog.notice("Ricevitore ESP-NOW pronto in modalità Light Sleep Automatico.");

  //Strategia del Trasmettitore: Il trasmettitore deve inviare lo
  //stesso pacchetto a raffica continua (in un ciclo for o while) per
  //una durata leggermente superiore al wake_interval del ricevitore
  //(nell'esempio, per almeno 210-250 millisecondi), finché non riceve
  //l'ACK di avvenuta consegna, garantendo così che il ricevitore
  //intercetti il messaggio durante i suoi 30ms di veglia.
  */


  /*
  //Se il server è sempre acceso e alimentato a rete,
  //l'approccio migliore per risparmiare il massimo dell'energia sul
  //ricevitore a batteria è la tecnica del Polling in Deep Sleep. Dato
  //che in Deep Sleep la radio è spenta e non può sentire nulla, il
  //ricevitore deve gestire la comunicazione. La logica funziona così:
  //  Il Ricevitore si sveglia dal Deep Sleep.
  //  Invia un breve messaggio al trasmettitore dicendo: "Sono sveglio, hai dati per me?".
  //  Rimane in ascolto con la radio accesa per una finestra temporale cortissima (es. 30-50 millisecondi).
  //  Il Trasmettitore (che è sempre acceso) riceve la richiesta e risponde immediatamente inviando i dati accumulati.
  //  Il ricevitore elabora i dati e torna istantaneamente in Deep Sleep per X minuti.
  */
  
  if (bootCount == 0 ){

    if (RESET_PAIR){
      frtosLog.warning(F("Reset pair information"));
      LittleFS.begin();
      LittleFS.format();
    }

    config.channel=1;
    config.accoppiato=false;
    
    frtosLog.notice(F("mounting FS..."));
    if (LittleFS.begin()) {
      frtosLog.notice(F("mounted LittleFS file system"));
      if(read_local_config()){
	bootCount = 1;
      }else{
	frtosLog.error(F("failed reading config file"));
      }
    } else {
      frtosLog.error(F("failed to mount FS"));
      frtosLog.warning(F("Reformat LittleFS"));
      LittleFS.begin();    
      LittleFS.format();
    }
  }

  frtosLog.notice("Boot number: %d accoppiato: %T", bootCount,config.accoppiato);

  if (bootCount > 0 and config.accoppiato){
    esp_wifi_set_channel(config.channel, WIFI_SECOND_CHAN_NONE);
    // Aggiunge il master come peer specifico
    
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, config.masterMac, 6);
    peerInfo.channel = 0;
    //peerInfo.ifidx = WIFI_IF_STA;  // Interfaccia usata (Station o AP)
    peerInfo.encrypt = false;         // enable with pioarduino only! tasmota configurated with no encryption
    memcpy(peerInfo.lmk, mio_lmk, 16);
    
    if (esp_now_is_peer_exist(peerInfo.peer_addr)){
      frtosLog.notice("boot peer broadcast già registrato");
    }else{    
      if (esp_now_add_peer(&peerInfo) == ESP_OK) {
	frtosLog.notice("boot Master registrato come peer fisso.");      
	frtosLog.notice("boot Accoppiamento riuscito!");
      }else{
	frtosLog.error("boot Error adding peer");
	config.accoppiato=false;
      }
    }
  }

  //Increment boot number and print it every reboot
  frtosLog.notice("Boot number: %d", bootCount);
  ++bootCount;
  
  /*
  First we configure the wake up source
  We set our ESP32 to wake up every 5 seconds
  */
  frtosLog.notice("Setup ESP32 to sleep for %d seconds",TIME_TO_SLEEP);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * S_TO_uS_FACTOR);

  /*
  Next we decide what all peripherals to shut down/keep on
  By default, ESP32 will automatically power down the peripherals
  not needed by the wakeup source, but if you want to be a poweruser
  this is for you. Read in detail at the API docs
  http://esp-idf.readthedocs.io/en/latest/api-reference/system/deep_sleep.html
  Left the line commented as an example of how to configure peripherals.
  The line below turns off all RTC peripherals in deep sleep.
  */
  //esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  //frtosLog.notice("Configured all RTC Peripherals to be powered down in sleep");
  
}
 
void loop() {

  // Create a struct_message called Readings to hold sensor readings
  message_data_crc outgoingMessage;

  if (!config.accoppiato or error_count > 10){
    frtosLog.notice("accoppiato %T  error count %d",config.accoppiato, error_count);

    config.channel++;
    if (config.channel >4) config.channel=2;
    frtosLog.notice("channel: %d",config.channel);
    esp_wifi_set_channel(config.channel, WIFI_SECOND_CHAN_NONE);
    delay(3000);
  }  
  
  if (config.accoppiato){

    transmissionCompleted = false;    
    outgoingMessage.type=99;
    // Set values to send
    outgoingMessage.message.datetime = now();
    outgoingMessage.message.temp = random(250,300);
    outgoingMessage.message.hum = random(1,100);
    outgoingMessage.message.pres = random(990,1030);
    outgoingMessage.crc = esp_rom_crc8_le(0, (const uint8_t*)&outgoingMessage.message, sizeof(outgoingMessage.message));
    
    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(config.masterMac, (uint8_t *) &outgoingMessage, sizeof(outgoingMessage));
    
    if (result == ESP_OK) {
      frtosLog.notice("Queued for send with success");
      //error_count=0;
    }
    else {
      frtosLog.notice("Error queueing the data");
      error_count++;
    }

    /*
      Now that we have setup a wake cause and if needed setup the
      peripherals state in deep sleep, we can now start going to
      deep sleep.
      In the case that no wake up sources were provided but deep
      sleep was started, it will sleep forever unless hardware
      reset occurs.
    */
    frtosLog.notice("Going to sleep now");
    Serial.flush();

    //ATTESA CRITICA: Aspetta che la callback OnDataSent venga eseguita
    unsigned long startTimeout = millis();
    while (!transmissionCompleted) {
      delay(10);
      // Timeout di sicurezza (es. 500ms) per evitare che l'ESP resti acceso all'infinito se il destinatario è spento
      if (millis() - startTimeout > 500) {
	frtosLog.notice("Timeout invio superato!");
	break;
      }
    }

    esp_deep_sleep_start();
    //delay(5000);
    frtosLog.notice("This will never be printed");    

    /*
    frtosLog.notice(F("RESTORE accoppiato: %T"),config.accoppiato);
    frtosLog.notice(F("RESTORE MAC 0: %X"),config.masterMac[0]);
    frtosLog.notice(F("RESTORE MAC 1: %X"),config.masterMac[1]);
    frtosLog.notice(F("RESTORE MAC 2: %X"),config.masterMac[2]);
    frtosLog.notice(F("RESTORE MAC 3: %X"),config.masterMac[3]);
    frtosLog.notice(F("RESTORE MAC 4: %X"),config.masterMac[4]);
    frtosLog.notice(F("RESTORE MAC 5: %X"),config.masterMac[5]);
    frtosLog.notice(F("RESTORE channel %d"),config.channel);
    
    esp_wifi_set_channel(config.channel, WIFI_SECOND_CHAN_NONE);
    // Aggiunge il master come peer specifico
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, config.masterMac, 6);
    peerInfo.channel = 0;
    //peerInfo.ifidx = WIFI_IF_STA;  // Interfaccia usata (Station o AP)
    peerInfo.encrypt = false;         // enable with pioarduino only! tasmota configurated with no encryption
    memcpy(peerInfo.lmk, mio_lmk, 16);
    
    if (esp_now_is_peer_exist(peerInfo.peer_addr)){
      frtosLog.notice("peer broadcast già registrato");
    }else{    
      if (esp_now_add_peer(&peerInfo) == ESP_OK) {
	frtosLog.notice("boot Master registrato come peer fisso.");      
	frtosLog.notice("boot Accoppiamento riuscito!");
      }else{
	frtosLog.error("boot Error adding peer");
	config.accoppiato=false;
      }
    }
    */
  }
}
