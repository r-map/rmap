#include "common.h"
#include "now_thread.h"


//***********************************************************************************************
//                         global definition to use in NOW callback
// pointers setted by class istance
now_data_t* nowThread::global_data=NULL;
//***********************************************************************************************


/*
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
*/

struct_config config;
const uint8_t broadcastAddress[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
volatile unsigned int last_state_update =0;
volatile state_t state = STATE_NONE;
volatile uint16_t seq=0;


// read configuration from EEPROM
bool read_local_config() {

  if (LittleFS.exists("/master.json")) {
    //file exists, read and load
    nowThread::global_data->logger->notice(F("now reading rmap config file"));
    File configFile = LittleFS.open("/master.json", "r");
    if (configFile) {
      nowThread::global_data->logger->notice(F("now opened master config file"));
      
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

	nowThread::global_data->logger->notice(F("now Config read:"));
	nowThread::global_data->logger->notice(F("now accoppiato: %T"),config.accoppiato);
	nowThread::global_data->logger->notice(F("now MAC 0: %X"),config.satelliteMac[0]);
	nowThread::global_data->logger->notice(F("now MAC 1: %X"),config.satelliteMac[1]);
	nowThread::global_data->logger->notice(F("now MAC 2: %X"),config.satelliteMac[2]);
	nowThread::global_data->logger->notice(F("now MAC 3: %X"),config.satelliteMac[3]);
	nowThread::global_data->logger->notice(F("now MAC 4: %X"),config.satelliteMac[4]);
	nowThread::global_data->logger->notice(F("now MAC 5: %X"),config.satelliteMac[5]);
	nowThread::global_data->logger->notice(F("now channel: %d"),config.channel);
	nowThread::global_data->logger->notice(F("now END config"));
	
	return true;
      } else {
	nowThread::global_data->logger->error(F("now reading master file: %s"),error.c_str());	
      }
    } else {
      nowThread::global_data->logger->warning(F("now master file do not exist"));
    }
  }
  return false;
}

// write configuration to EEPROM
bool write_local_config() {

  //save the custom parameters to FS
  nowThread::global_data->logger->notice(F("now saving master config"));
  
  File configFile = LittleFS.open("/master.json", "w");
  if (!configFile) {
    nowThread::global_data->logger->error(F("now failed to open rmap config file for writing"));
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
  nowThread::global_data->logger->notice(F("now saved master config parameter"));
  //end save
  return true;
}

void nowThread::add_broadcast_peer(){
  // Register peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  //peerInfo.ifidx = WIFI_IF_STA;  // Interfaccia usata (Station o AP)
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  if (esp_now_is_peer_exist(peerInfo.peer_addr)){
    data->logger->notice(F("now peer broadcast already registered"));
  }else{    
    // Add peer        
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
      data->logger->error(F("now Failed to add broadcast peer"));
    }
  }
}


// Callback when data is sent
void OnDataSent(const  uint8_t *des_addr, esp_now_send_status_t status) {
  nowThread::global_data->logger->notice(F("now OnDataSent"));
  nowThread::global_data->logger->notice(F("now State: %d"),state);
  nowThread::global_data->logger->notice(F("now destination MAC: %X:%X:%X:%X:%X:%X"),
		  des_addr[0], des_addr[1], des_addr[2],
		  des_addr[3], des_addr[4], des_addr[5]);
  nowThread::global_data->logger->notice(F("now Last Packet Send Status: %s"), status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail"  );
  last_state_update=millis();
  if (status != ESP_NOW_SEND_SUCCESS){
    state=STATE_NONE;
    nowThread::global_data->logger->error(F("now Error sending"));
  }
  nowThread::global_data->logger->notice(F("State: %d"),state);
}

// Callback when data is received
void OnDataRecv(const esp_now_recv_info_t *esp_now_info, const uint8_t *incomingData, int len) {
  // Create a message_pair to hold incoming sensor readings
  nowThread::global_data->logger->notice(F("now Packed received from MAC: : %X:%X:%X:%X:%X:%X"),
		  esp_now_info->src_addr[0], esp_now_info->src_addr[1], esp_now_info->src_addr[2],
		  esp_now_info->src_addr[3], esp_now_info->src_addr[4], esp_now_info->src_addr[5]);
  nowThread::global_data->logger->notice(F("now Bytes received: %d"),len);

  // Controlla se è una risposta al Pairing
  uint16_t type;
  memcpy(&type, incomingData, sizeof(type));
  if (type == 1 ) {
    nowThread::global_data->logger->notice(F("now broadcast ack received"));
    message_pair_crc incomingMessage;
    memcpy(&incomingMessage, incomingData, len);
    uint8_t crc = esp_rom_crc8_le(0, (const uint8_t*)&incomingMessage.message,sizeof(incomingMessage.message));
    nowThread::global_data->logger->notice(F("now computed CRC: %d"),crc);
    if (crc != incomingMessage.crc){
      nowThread::global_data->logger->error(F("now CRC mismatch"));
      return;
    }
    
    if (seq+1 == incomingMessage.message.seq){
      nowThread::global_data->logger->notice(F("now SEQ: %d"),incomingMessage.message.seq);
    }else{
      nowThread::global_data->logger->error(F("now SEQ mismatch: %d, %d"),incomingMessage.message.seq,seq+1);
      return;
    }

    if (state != STATE_PAIR_SENDED){
      nowThread::global_data->logger->error(F("now STATE mismatch: %d, %d"),state, STATE_PAIR_SENDED);
      state = STATE_NONE;
      return;
    }

    if (config.accoppiato){
      nowThread::global_data->logger->error(F("now PAIR mismatch"));
      return;
    }

    if ((millis() - last_state_update) > TRANSACTION_TIMEOUT){
      nowThread::global_data->logger->error(F("now Transaction timeout %d"),millis() - last_state_update);
      state=STATE_NONE;
      return;
    }
    
    state=STATE_PAIR_ACK_RECEIVED;

    if (esp_now_is_peer_exist(esp_now_info->src_addr)){
      nowThread::global_data->logger->notice(F("now peer already registered"));
    }else{    
      // Aggiunge il satellite come peer specifico
      esp_now_peer_info_t peerInfo = {};
      memcpy(peerInfo.peer_addr, esp_now_info->src_addr, 6);
      peerInfo.channel = 0;
      //peerInfo.ifidx = WIFI_IF_STA;  // Interfaccia usata (Station o AP)
      peerInfo.encrypt = false;         // enable with pioarduino only! tasmota configurated with no encryption
      //memcpy(peerInfo.lmk, mio_lmk, 16);
      
      if (esp_now_add_peer(&peerInfo) == ESP_OK) {
	nowThread::global_data->logger->notice(F("now Satellite registered"));      
	nowThread::global_data->logger->notice(F("now Paired!"));
	memcpy(config.satelliteMac, esp_now_info->src_addr, 6); // Salva il MAC reale del satellite

	// Risponde al trasmettitore per confermare il pairing
	// Create a message_pair called Readings to hold sensor readings
	message_pair_crc outgoingMessage;
	outgoingMessage.message.type=2;
	outgoingMessage.message.seq=incomingMessage.message.seq+1;
	outgoingMessage.message.datetime=now();
	outgoingMessage.crc = esp_rom_crc8_le(0, (const uint8_t*)&outgoingMessage.message, sizeof(outgoingMessage.message));
	esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &outgoingMessage, sizeof(outgoingMessage));
	if (result == ESP_OK) {
	  state=STATE_PAIR_DONE;
	  nowThread::global_data->logger->notice(F("now Sent with success"));
	} else {
	  state=STATE_NONE;
	  nowThread::global_data->logger->error(F("now Sent with error"));
	  return;
	}

	config.accoppiato=true;
	esp_now_del_peer(broadcastAddress);
	
	if (!write_local_config())nowThread::global_data->logger->error(F("now Error writing config file"));
      }else{
	nowThread::global_data->logger->error(F("now Error adding peer"));
      }
    }
  } else if (type == 99) {

    message_data_crc incomingMessage;
    memcpy(&incomingMessage, incomingData, len);
    uint8_t crc = esp_rom_crc8_le(0, (const uint8_t*)&incomingMessage.message,sizeof(incomingMessage.message));
    if (crc != incomingMessage.crc){
      nowThread::global_data->logger->error(F("now CRC mismatch"));
      return;
    }

    if (!config.accoppiato){
      nowThread::global_data->logger->error(F("now PAIR mismatch"));
      return;
    }

    if (state != STATE_NONE and state != STATE_PAIR_DONE and state != STATE_DATA_DONE){
      nowThread::global_data->logger->error(F("now STATE mismatch: %d, %d/%d/%d"),state, STATE_NONE,STATE_PAIR_DONE,STATE_DATA_DONE);
      return;
    }

    state=STATE_DATA_RECEIVED;

    char dt[DATE_TIME_STRING_LENGTH];
    snprintf(dt, DATE_TIME_STRING_LENGTH, "%04u-%02u-%02uT%02u:%02u:%02u", year(), month(), day(), hour(), minute(), second());

    nowThread::global_data->logger->notice(F("now topic received: %s"),incomingMessage.message.mqttmessage.topic);
    nowThread::global_data->logger->notice(F("now payload received: %s"),incomingMessage.message.mqttmessage.payload);

    if (enqueueMqttMessage(incomingMessage.message.mqttmessage)){
      message_pair_crc outgoingMessage;
      outgoingMessage.message.type=3;                     // data ACK
      outgoingMessage.message.seq=incomingMessage.message.seq+1;
      outgoingMessage.message.datetime=now();
      outgoingMessage.crc = esp_rom_crc8_le(0, (const uint8_t*)&outgoingMessage.message, sizeof(outgoingMessage.message));
      nowThread::global_data->logger->notice(F("now computed CRC: %d"),outgoingMessage.crc);
      esp_err_t result = esp_now_send(config.satelliteMac, (uint8_t *) &outgoingMessage, sizeof(outgoingMessage));
      if (result == ESP_OK) {
	state=STATE_DATA_DONE;
	nowThread::global_data->logger->notice(F("now Sent with success"));
      } else {
	state=STATE_NONE;
	nowThread::global_data->logger->error(F("now Sent with error"));
      }
    }else{

      // TODO NACK
      
    }
  }
}

// encode and enqueue in a proper queue one message
bool enqueueMqttMessage(const mqttMessage_t mqtt_message) {
  
  bool rc=true;
  //mqtt_message.sent=0;
  
  nowThread::global_data->logger->notice(F("now have to publish:"));
  nowThread::global_data->logger->notice(F("now Topic: %s"),mqtt_message.topic);
  nowThread::global_data->logger->notice(F("now Payload: %s"),mqtt_message.payload);
        
  // if there are enough space left on the publish queue send it
  if (nowThread::global_data->mqttqueue->NumSpacesLeft() > QUEUE_SPACELEFT_MEASURE){
    nowThread::global_data->logger->notice(F("now enqueue for mqtt: %s ; %s"),  mqtt_message.topic, mqtt_message.payload);    
    if(!nowThread::global_data->mqttqueue->Enqueue(&mqtt_message,pdMS_TO_TICKS(0))){
      nowThread::global_data->logger->error(F("now enqueue for mqtt: %s ; %s"),  mqtt_message.topic, mqtt_message.payload);
      if (nowThread::global_data->dbqueue->Enqueue(&mqtt_message,pdMS_TO_TICKS(0))){      // on error send il to DB
	nowThread::global_data->logger->notice(F("now enqueue for db"));
      }else{
	nowThread::global_data->logger->error(F("now lost message for db: %s ; %s"),  mqtt_message.topic, mqtt_message.payload);
	rc=false;
      }
    }
  } else {    // if there are no enough space left on the publish queue send it to the archive
    if(nowThread::global_data->dbqueue->Enqueue(&mqtt_message,pdMS_TO_TICKS(0))){
      nowThread::global_data->logger->notice(F("now enqueue for db"));
    }else{
      nowThread::global_data->logger->error(F("now lost message for db: %s ; %s"),  mqtt_message.topic, mqtt_message.payload);
      rc=false;
    }
  }
  return rc;
}

nowThread::nowThread(now_data_t* now_data)
  : Thread{"now", TASK_NOW_STACK_SIZE, TASK_NOW_PRIORITY
           # if portNUM_PROCESSORS > 1
	   ,1  // if multicore 1 indicate the index number of the CPU which the task should be pinned to
           #endif
          },
    data{now_data}
{
  //data->logger->notice("Create Thread %s %d", GetName().c_str(), data->id);

  //data->status->no_heap_memory=ok;

  global_data=data;
  
  //Start();
  
};

nowThread::~nowThread()
{
}


void nowThread::Begin()
{
  // slow down
  //setCpuFrequencyMhz(80);

  /*
  if (RESET_PAIR){
    data->logger->warning(F("now Reset pair information"));
    LittleFS.begin();
    LittleFS.format();
  }
  */
  
  data->logger->notice(F("now Started"));

  if(!read_local_config()) data->logger->error(F("now failed reading config file"));
  
  data->logger->notice(F("now channel: %d"),config.channel);
  esp_wifi_set_channel(config.channel, WIFI_SECOND_CHAN_NONE);

  // ESP-Now Range Test: Real-World Results for ESP32 Devices
  // https://youtu.be/oz0a7Ur7nko?si=aUMBJ4SpeXTSMMPg
  /*
  esp_err_t err = esp_wifi_set_protocol(WIFI_IF_STA,WIFI_PROTOCOL_LR);
  if (err == ESP_OK) {
    data->logger->notice(F("now Protocol successfully restricted to Long Range (LR) Mode successfully enabled!"));
  } else {
    data->logger->error(F("now Failed to set protocol. Error code: %d"), err);
  }
  */
  
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    data->logger->notice(F("now Error initializing ESP-NOW"));
    return;
  }

  // Imposta la PMK globale
  // Se questa funzione fallisce o non viene chiamata, ESP-NOW userà una PMK di default della mesh,
  // compromettendo la sicurezza dell'intero ecosistema.
  /*
  if (esp_now_set_pmk(mia_pmk) == ESP_OK) {
    data->logger->notice(F("now PMK impostata con successo!"));
  } else {
    data->logger->error(F("now Errore nell'impostazione della PMK"));
    return;
  }
  */
  
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
    //memcpy(peerInfo.lmk, mio_lmk, 16);
    
    if (esp_now_is_peer_exist(peerInfo.peer_addr)){
      data->logger->notice(F("now boot peer broadcast già registrato"));
    }else{    
      if (esp_now_add_peer(&peerInfo) == ESP_OK) {
	data->logger->notice(F("now boot Satellite registrato come peer fisso."));      
	data->logger->notice(F("now boot Accoppiamento riuscito!"));
      }else{
	data->logger->error(F("now boot Error adding peer"));
	config.accoppiato=false;
      }
    }
  }else{
    add_broadcast_peer();
  }  
}

void nowThread::Cleanup()
{
  data->logger->notice(F("now Delete Thread %s %d"), GetName().c_str(), data->id);
  // todo disconnect and others
  //data->status->no_heap_memory=unknown;
  delete this;
}

void nowThread::Run() {
  data->logger->notice(F("now Starting Thread %s %d"), GetName().c_str(), data->id);
  for(;;){

    delay(1000);

    if (!config.accoppiato){
      
      // send pairing request
      message_pair_crc outgoingMessage;
      outgoingMessage.message.type=0;
      outgoingMessage.message.seq=++seq;
      outgoingMessage.message.datetime=now();
      outgoingMessage.crc = esp_rom_crc8_le(0, (const uint8_t*)&outgoingMessage.message, sizeof(outgoingMessage.message));
      data->logger->notice(F("now State: %d"),state);
      data->logger->notice(F("now Send broadcast message"));
      data->logger->notice(F("now computed CRC: %d"),outgoingMessage.crc);
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &outgoingMessage, sizeof(outgoingMessage));
      if (result == ESP_OK) {
	state=STATE_PAIR_SENDED;
	data->logger->notice(F("now Pairing request sent with success"));
      } else {
	state=STATE_NONE;
	data->logger->error(F("now Error sending pairing request"));
      }    
    }
  }
};
  
