#include "common.h"
#include "now_sat_thread.h"

//***********************************************************************************************
//                         global definition to use in NOW callback
// pointers setted by class istance
now_sat_data_t* nowSatThread::global_data=NULL;
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

static const uint8_t broadcastAddress[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
static RTC_DATA_ATTR int bootCount;
static RTC_DATA_ATTR now_config_t config;
static RTC_DATA_ATTR volatile uint8_t error_count;
static RTC_DATA_ATTR volatile unsigned int last_state_update;
static RTC_DATA_ATTR volatile state_sat_t state;
static RTC_DATA_ATTR volatile uint16_t seq;
// Flag per verificare se l'invio è completato
static RTC_DATA_ATTR volatile bool transmissionCompleted ;
// Flag per sapere se il canale è cambiato rispetto alla conf salvata
static RTC_DATA_ATTR volatile bool channelchanged;

// read configuration from EEPROM
static bool read_local_config() {

  if (LittleFS.exists("/satellite.json")) {
    //file exists, read and load
    nowSatThread::global_data->logger->notice(F("nowsat reading rmap config file"));
    File configFile = LittleFS.open("/master.json", "r");
    if (configFile) {
      nowSatThread::global_data->logger->notice(F("nowsat opened master config file"));
      
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
	JsonArray mac = doc["mastermac"];
	config.peerMac[0]= mac[0]; // 1
	config.peerMac[1]= mac[1]; // 2
	config.peerMac[2]= mac[2]; // 3
	config.peerMac[3]= mac[3]; // 4
	config.peerMac[4]= mac[4]; // 5
	config.peerMac[5]= mac[5]; // 6
	config.channel = doc["channel"];

	nowSatThread::global_data->logger->notice(F("nowsat Config read:"));
	nowSatThread::global_data->logger->notice(F("nowsat accoppiato: %T"),config.accoppiato);
	nowSatThread::global_data->logger->notice(F("nowsat MAC 0: %X"),config.peerMac[0]);
	nowSatThread::global_data->logger->notice(F("nowsat MAC 1: %X"),config.peerMac[1]);
	nowSatThread::global_data->logger->notice(F("nowsat MAC 2: %X"),config.peerMac[2]);
	nowSatThread::global_data->logger->notice(F("nowsat MAC 3: %X"),config.peerMac[3]);
	nowSatThread::global_data->logger->notice(F("nowsat MAC 4: %X"),config.peerMac[4]);
	nowSatThread::global_data->logger->notice(F("nowsat MAC 5: %X"),config.peerMac[5]);
	nowSatThread::global_data->logger->notice(F("nowsat channel: %d"),config.channel);
	nowSatThread::global_data->logger->notice(F("nowsat END config"));
	
	return true;
      } else {
	nowSatThread::global_data->logger->error(F("nowsat reading master file: %s"),error.c_str());	
      }
    } else {
      nowSatThread::global_data->logger->warning(F("nowsat master file do not exist"));
    }
  }
  return false;
}

// write configuration to EEPROM
static bool write_local_config() {

  //save the custom parameters to FS
  nowSatThread::global_data->logger->notice(F("nowsat saving master config"));
  
  File configFile = LittleFS.open("/satellite.json", "w");
  if (!configFile) {
    nowSatThread::global_data->logger->error(F("nowsat failed to open rmap config file for writing"));
    return false;
  }

  DynamicJsonDocument doc(200); 
  doc["ver"] = SOFTWARE_VERSION;
  doc["accoppiato"] = config.accoppiato;
  doc["mastermac"][0] = config.peerMac[0];
  doc["mastermac"][1] = config.peerMac[1];
  doc["mastermac"][2] = config.peerMac[2];
  doc["mastermac"][3] = config.peerMac[3];
  doc["mastermac"][4] = config.peerMac[4];
  doc["mastermac"][5] = config.peerMac[5];

  doc["channel"] = config.channel;
  char buffer[256];
  serializeJson(doc, buffer, sizeof(buffer));
  configFile.print(buffer);
  configFile.close();
  nowSatThread::global_data->logger->notice(F("nowsat saved master config parameter"));
  //end save
  return true;
}

// Callback when data is sent
static void OnDataSent(const  uint8_t *des_addr, esp_now_send_status_t status) {
  nowSatThread::global_data->logger->notice(F("nowsat OnDataSent"));
  nowSatThread::global_data->logger->notice(F("nowsat State: %d"),state);
  nowSatThread::global_data->logger->notice(F("nowsat destination MAC: %X:%X:%X:%X:%X:%X"),
		  des_addr[0], des_addr[1], des_addr[2],
		  des_addr[3], des_addr[4], des_addr[5]);
  nowSatThread::global_data->logger->notice("nowsat Last Packet Send Status: %s", status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail"  );
  last_state_update=millis();
  if (status != ESP_NOW_SEND_SUCCESS){
    error_count++;
    state=STATE_SAT_NONE;
    nowSatThread::global_data->logger->error("nowsat Error sending");
  }else{
    error_count=0;
  }
  nowSatThread::global_data->logger->notice(F("nowsat State: %d"),state);
}

// Callback when data is received
static void OnDataRecv(const esp_now_recv_info_t *esp_now_info, const uint8_t *incomingData, int len) {
  // Create a struct_message to hold incoming sensor readings
  nowSatThread::global_data->logger->notice(F("nowsat Pacchetto ricevuto da MAC: : %X:%X:%X:%X:%X:%X"),
		  esp_now_info->src_addr[0], esp_now_info->src_addr[1], esp_now_info->src_addr[2],
		  esp_now_info->src_addr[3], esp_now_info->src_addr[4], esp_now_info->src_addr[5]);
  nowSatThread::global_data->logger->notice(F("nowsat Bytes received: %d"),len);
  
  // Controlla se è una richiesta di Pairing
  uint16_t type;
  memcpy(&type, incomingData, sizeof(type));
  if (type == 0) {
    nowSatThread::global_data->logger->notice("nowsat Richiesta di pairing ricevuta");
    message_pair_crc incomingMessage;
    memcpy(&incomingMessage, incomingData, len);

    uint8_t crc = esp_rom_crc8_le(0, (const uint8_t*)&incomingMessage.message, sizeof(incomingMessage.message));
    nowSatThread::global_data->logger->notice(F("nowsat computed CRC: %d"),crc);

    if (crc != incomingMessage.crc){
      nowSatThread::global_data->logger->error("nowsat CRC mismatch");
      nowSatThread::global_data->logger->error("nowsat crcc=%d  crcr=%d",crc,incomingMessage.crc);
      return;
    }

    if (state != STATE_SAT_NONE){
      nowSatThread::global_data->logger->error(F("nowsat STATE mismatch: %d, %d"),state, STATE_SAT_NONE);
      state = STATE_SAT_NONE;
      return;
    }

    if (config.accoppiato){
      nowSatThread::global_data->logger->error("nowsat PAIR mismatch");
      return;
    }
    
    nowSatThread::global_data->logger->notice(F("nowsat SEQ: %d"),incomingMessage.message.seq);
    seq=incomingMessage.message.seq;
    last_state_update=millis();
    state = STATE_SAT_PAIR_RECEIVED;
    // Risponde al trasmettitore per confermare il pairing
    // Create a struct_message called Readings to hold sensor readings
    message_pair_crc outgoingMessage;
    outgoingMessage.message.type=1;
    outgoingMessage.message.seq=++seq;
    outgoingMessage.message.datetime=now();
    outgoingMessage.crc = esp_rom_crc8_le(0, (const uint8_t*)&outgoingMessage.message, sizeof(outgoingMessage.message));

    add_broadcast_peer();
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &outgoingMessage, sizeof(outgoingMessage));
    if (result == ESP_OK) {
      state=STATE_SAT_PAIR_ACK_SENDED;
      nowSatThread::global_data->logger->notice("nowsat Sent with success");
    } else {
      state=STATE_SAT_NONE;
      nowSatThread::global_data->logger->error("nowsat Sent with error");
      return;
    }

  } else if (type == 2 ) {
    nowSatThread::global_data->logger->notice("nowsat ACK al broadcast ricevuta");
    message_pair_crc incomingMessage;
    memcpy(&incomingMessage, incomingData, len);
    uint8_t crc = esp_rom_crc8_le(0, (const uint8_t*)&incomingMessage.message, sizeof(incomingMessage.message));
    if (crc != incomingMessage.crc){
      nowSatThread::global_data->logger->error("nowsat CRC mismatch");
      return;
    }

    if ((millis() - last_state_update) > TRANSACTION_TIMEOUT){
      nowSatThread::global_data->logger->error(F("nowsat Transaction timeout %d"),millis() - last_state_update);
      state=STATE_SAT_NONE;
    }
    
    if (state != STATE_SAT_PAIR_ACK_SENDED){
      nowSatThread::global_data->logger->error(F("nowsat STATE mismatch: %d, %d"),state, STATE_SAT_PAIR_ACK_SENDED);
      state = STATE_SAT_NONE;
      return;
    }

    if (seq+1 == incomingMessage.message.seq){
      nowSatThread::global_data->logger->notice(F("nowsat SEQ: %d"),incomingMessage.message.seq);
    }else{
      nowSatThread::global_data->logger->error(F("nowsat SEQ mismatch: %d, %d"),incomingMessage.message.seq,seq+1);
      return;
    }
    
    if (esp_now_is_peer_exist(esp_now_info->src_addr)){
      nowSatThread::global_data->logger->notice("nowsat peer già registrato");
    }else{
      // Aggiunge il master come peer specifico
      esp_now_peer_info_t peerInfo = {};
      memcpy(peerInfo.peer_addr, esp_now_info->src_addr, 6);
      peerInfo.channel = 0;
      //peerInfo.ifidx = WIFI_IF_STA;  // Interfaccia usata (Station o AP)
      peerInfo.encrypt = false;        // enable with pioarduino only! tasmota configurated with no encryption
      //memcpy(peerInfo.lmk, mio_lmk, 16);
      
      if (esp_now_add_peer(&peerInfo) == ESP_OK) {
	nowSatThread::global_data->logger->notice("nowsat Master registrato come peer fisso.");      
	nowSatThread::global_data->logger->notice("nowsat Accoppiamento riuscito!");
	memcpy(config.peerMac, esp_now_info->src_addr, 6); // Salva il MAC reale del master
	error_count=0;
	//Rimuove il peer broadcast generico
	esp_now_del_peer(broadcastAddress);
	if (!write_local_config())nowSatThread::global_data->logger->error("nowsat Error writing config file");
      }else{
	nowSatThread::global_data->logger->notice("nowsat Error adding peer");
      }
    }

    state = STATE_SAT_PAIR_DONE;
    
  } else if (type == 3 ) {
    nowSatThread::global_data->logger->notice("nowsat ACK ai dati ricevuta");
    message_pair_crc incomingMessage;
    memcpy(&incomingMessage, incomingData, len);
    uint8_t crc = esp_rom_crc8_le(0, (const uint8_t*)&incomingMessage.message, sizeof(incomingMessage.message));
    if (crc != incomingMessage.crc){
      nowSatThread::global_data->logger->error("nowsat CRC mismatch");
      return;
    }

    if ((millis() - last_state_update) > TRANSACTION_TIMEOUT){
      nowSatThread::global_data->logger->error(F("nowsat Transaction timeout %d"),millis() - last_state_update);
      state=STATE_SAT_NONE;
    }
    
    if (state != STATE_SAT_DATA_SENDED){
      nowSatThread::global_data->logger->error(F("nowsat STATE mismatch: %d, %d"),state, STATE_SAT_DATA_SENDED);
      state = STATE_SAT_NONE;
      return;
    }
    
    if (seq+1 == incomingMessage.message.seq){
      nowSatThread::global_data->logger->notice(F("nowsat SEQ: %d"),incomingMessage.message.seq);
      // TODO dequeue the message when we will have queue
    }else{
      nowSatThread::global_data->logger->error(F("nowsat SEQ mismatch: %d, %d"),incomingMessage.message.seq,seq+1);
    }

    // Sblocca il ciclo principale consentendo il deep sleep
    transmissionCompleted = true;
    if (channelchanged){
      if (!write_local_config())nowSatThread::global_data->logger->error("nowsat Error writing config file");
    }
    channelchanged=false;
    setTime(incomingMessage.message.datetime);
    state = STATE_SAT_DATA_DONE;
  } else {
    nowSatThread::global_data->logger->error("nowsat message type unknown");
    return;
  }
}

static void add_broadcast_peer(){
  // Register peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  //peerInfo.ifidx = WIFI_IF_STA;  // Interfaccia usata (Station o AP)
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  if (esp_now_is_peer_exist(peerInfo.peer_addr)){
    nowSatThread::global_data->logger->notice(F("nowsat now peer broadcast already registered"));
  }else{    
    // Add peer        
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
      nowSatThread::global_data->logger->error(F("nowsat now Failed to add broadcast peer"));
    }
  }
}

nowSatThread::nowSatThread(now_sat_data_t* now_sat_data)
  : Thread{"now sat", TASK_NOW_SAT_STACK_SIZE, TASK_NOW_SAT_PRIORITY
           # if portNUM_PROCESSORS > 1
	   ,1  // if multicore 1 indicate the index number of the CPU which the task should be pinned to
           #endif
          },
    data{now_sat_data}
{
  //data->logger->notice("nowsat Create Thread %s %d", GetName().c_str(), data->id);
  //data->status->no_heap_memory=ok;

  global_data=data;
  
  //Start();
  
};

nowSatThread::~nowSatThread()
{
}

void nowSatThread::Begin()
{
  if (bootCount == 0 ){

    bootCount = 0;
    config.channel=1;
    config.accoppiato=false;
    error_count=0;
    last_state_update = 0;
    state = STATE_SAT_NONE;
    seq=0;
    transmissionCompleted = false ;
    channelchanged = false;
    
    uintptr_t start = (uintptr_t)&_rtc_data_start;
    uintptr_t end   = (uintptr_t)&_rtc_data_end;
    data->logger->notice(F("nowsat RTC used data: %d bytes on 8192 total"), (unsigned)(end - start));
  }
  
  // ESP-Now Range Test: Real-World Results for ESP32 Devices
  // https://youtu.be/oz0a7Ur7nko?si=aUMBJ4SpeXTSMMPg
  /*
  esp_err_t err = esp_wifi_set_protocol(WIFI_IF_STA,WIFI_PROTOCOL_LR);
  if (err == ESP_OK) {
    data->logger->notice("nowsat Protocol successfully restricted to Long Range (LR) Mode successfully enabled!");
  } else {
    data->logger->notice("nowsat Failed to set protocol. Error code: %d", err);
  }
  */
  
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    data->logger->notice("nowsat Error initializing ESP-NOW");
    return;
  }

  /*
  // Imposta la PMK globale
  // Se questa funzione fallisce o non viene chiamata, ESP-NOW userà una PMK di default della mesh,
  // compromettendo la sicurezza dell'intero ecosistema.
  if (esp_now_set_pmk(mia_pmk) == ESP_OK) {
    data->logger->notice("nowsat PMK impostata con successo!");
  } else {
    data->logger->notice("nowsat Errore nell'impostazione della PMK");
    return;
  }
  */
  
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(esp_now_send_cb_t(OnDataSent));
  
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
  
  if (bootCount == 0 ){

    if(read_local_config()){
      bootCount = 1;
    }else{
      data->logger->error(F("nowsat failed reading config file"));
    }
  }

  data->logger->notice("nowsat Boot number: %d accoppiato: %T", bootCount,config.accoppiato);

  if (bootCount > 0 and config.accoppiato){
    esp_wifi_set_channel(config.channel, WIFI_SECOND_CHAN_NONE);
    // Aggiunge il master come peer specifico
    
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, config.peerMac, 6);
    peerInfo.channel = 0;
    //peerInfo.ifidx = WIFI_IF_STA;  // Interfaccia usata (Station o AP)
    peerInfo.encrypt = false;         // enable with pioarduino only! tasmota configurated with no encryption
    //memcpy(peerInfo.lmk, mio_lmk, 16);
    
    if (esp_now_is_peer_exist(peerInfo.peer_addr)){
      data->logger->notice("nowsat boot peer broadcast già registrato");
    }else{    
      if (esp_now_add_peer(&peerInfo) == ESP_OK) {
	data->logger->notice("nowsat boot Master registrato come peer fisso.");      
	data->logger->notice("nowsat boot Accoppiamento riuscito!");
	config.accoppiato=true;
      }else{
	data->logger->error("nowsat boot Error adding peer");
	config.accoppiato=false;
      }
    }
  }

  //Increment boot number and print it every reboot
  data->logger->notice("nowsat Boot number: %d", bootCount);
  ++bootCount;
  
  /*
  First we configure the wake up source
  We set our ESP32 to wake up every 5 seconds
  */
  data->logger->notice("nowsat Setup ESP32 to sleep for %d seconds",TIME_TO_SLEEP);
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
  //data->logger->notice("nowsat Configured all RTC Peripherals to be powered down in sleep");
}

void nowSatThread::Cleanup()
{
  data->logger->notice(F("nowsat Delete Thread %s %d"), GetName().c_str(), data->id);
  // todo disconnect and others
  //data->status->no_heap_memory=unknown;
  delete this;
}

void nowSatThread::Run() {
  data->logger->notice(F("nowsat  Starting Thread %s %d"), GetName().c_str(), data->id);
  for(;;){

    // if there are no enough space left on the mqtt queue send it to the DB
    while (data->mqttqueue->NumSpacesLeft() <= QUEUE_SPACELEFT_PUBLISH){
      store();
    }

    mqttMessage_t message;
    data->mqttqueue->Peek(&message, pdMS_TO_TICKS( 1000 ));     // wait for a new incoming message
    while(data->mqttqueue->Peek(&message, pdMS_TO_TICKS( 0 ))){
      if (!doRelay(message)) break;  	// publish message
      data->mqttqueue->Dequeue(&message, pdMS_TO_TICKS( 0 ));
    }
    while (data->recoveryqueue->Peek(&message, pdMS_TO_TICKS( 0 ))){
      if (message.sent==0 and message.topic[0] == NULL and message.payload[0] == NULL){
	data->recoveryqueue->Dequeue(&message, pdMS_TO_TICKS( 0 ));
	if(!data->dbqueue->Enqueue(&message,pdMS_TO_TICKS(0))){
	  data->logger->error(F("nowsat lost SYNC message for db: %s ; %s"),  message.topic, message.payload);
	}else{
	  data->logger->notice(F("nowsat SYNC message queued for db"));
	}
      }else{
	if (!doRelay(message,true)) break; 	// publish message
	data->recoveryqueue->Dequeue(&message, pdMS_TO_TICKS( 0 ));
	while(data->mqttqueue->Peek(&message, pdMS_TO_TICKS( 0 ))){
	  if (!doRelay(message)) break;  	// publish message
	  data->mqttqueue->Dequeue(&message, pdMS_TO_TICKS( 0 ));
	}
      }
    }
    
    data->logger->notice(F("nowsat mqtt     queue space left %d"),data->mqttqueue->NumSpacesLeft());
    data->logger->notice(F("nowsat recovery queue space left %d"),data->recoveryqueue->NumSpacesLeft());
    
    /*
      Now that we have setup a wake cause and if needed setup the
      peripherals state in deep sleep, we can now start going to
      deep sleep.
      In the case that no wake up sources were provided but deep
      sleep was started, it will sleep forever unless hardware
      reset occurs.
    */
    
    //data->logger->notice("nowsat I am going to sleep now");
    //Serial.flush();
    //
    // esp_deep_sleep_start();
    // delay(TIME_TO_SLEEP*1000);   // as alternative to sleep
    // data->logger->notice(F("nowsat This will never be printed in DEEP sleep mode"));
    
  }
}
// get one message from publish queue and send it to the queue for DB
// but if it is a resend message (sent == true) skip it;
// now we have a separate queue for recovery so the check is no more required
void nowSatThread::store() {
  
  mqttMessage_t mqtt_message;

  if (data->mqttqueue->Dequeue(&mqtt_message, pdMS_TO_TICKS( 0 ))){;  // dequeue
    if (mqtt_message.sent){
      data->logger->error(F("nowsat skip and do not store message sended before: %s ; %s"), mqtt_message.topic, mqtt_message.payload);
    }else{
      if(data->dbqueue->Enqueue(&mqtt_message,pdMS_TO_TICKS(0))){
	data->logger->notice(F("nowsat skip and enqueue message for db: %s ; %s"), mqtt_message.topic, mqtt_message.payload);
      }else{
	data->logger->error(F("nowsat lost message for db: %s ; %s"), mqtt_message.topic, mqtt_message.payload);
      }
    }
  }else{
    data->logger->error(F("nowsat getting message from mqtt queue"));
  }
}

// try to send message to the master and send the message to the DB queue
// only if it is required
// set recovery to true if the message come from recovery queue
bool nowSatThread::doRelay(mqttMessage_t mqtt_message, const bool recovery) {

  bool rc=false;
  bool resend = mqtt_message.sent;
  
  if(nowPublish(mqtt_message)){
    mqtt_message.sent=1;  // all done: flag as sent
    rc=true;
  }

  // if it was already sendend skip it and do do not store
  // if publish fail and come from recovery skip it and do do not store
  if (not resend and not (rc == false and recovery)) {
    if(!data->dbqueue->Enqueue(&mqtt_message,pdMS_TO_TICKS(0))){
      data->logger->error(F("nowsat lost message for db: %s ; %s"),  mqtt_message.topic, mqtt_message.payload);
    }
  }
  
  return rc;
}

bool nowSatThread::nowPublish(mqttMessage_t mqtt_message) {

  bool rc=false;
  
  if(state == STATE_SAT_PAIR_DONE) state = STATE_SAT_NONE;
    
  if (!config.accoppiato or error_count > 10){
    data->logger->notice(F("nowsat accoppiato %T  error count %d"),config.accoppiato, error_count);
    
    config.channel++;
    channelchanged=true;
    if (config.channel >13) config.channel=1;
    data->logger->notice("nowsat channel: %d",config.channel);
    esp_wifi_set_channel(config.channel, WIFI_SECOND_CHAN_NONE);
    if (!config.accoppiato) delay(3000);
  }  
  
  if (!config.accoppiato) return rc;
  
  if (state != STATE_SAT_NONE and state != STATE_SAT_DATA_DONE){
    data->logger->notice(F("nowsat STATE not ready: %d, %d"),state, STATE_SAT_NONE);
    state != STATE_SAT_NONE;
    return rc;
  }
  
  transmissionCompleted = false;    
  // Create a struct_message called Readings to hold sensor readings
  message_data_crc outgoingMessage;
  outgoingMessage.message.type=99;
  outgoingMessage.message.seq = ++seq;
  
  // Set values to send
  outgoingMessage.message.mqttmessage=mqtt_message;
  
  outgoingMessage.crc = esp_rom_crc8_le(0, (const uint8_t*)&outgoingMessage.message, sizeof(outgoingMessage.message));
    
  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(config.peerMac, (uint8_t *) &outgoingMessage, sizeof(outgoingMessage));
  if (result == ESP_OK) {
    data->logger->notice("nowsat Queued for send with success");
    //error_count=0;
    state = STATE_SAT_DATA_SENDED;
    rc=true;

    //ATTESA CRITICA: Aspetta che la callback OnDataSent venga eseguita
    unsigned long startTimeout = millis();
    while (!transmissionCompleted) {
      delay(10);
      // Timeout di sicurezza (es. 1000ms) per evitare che l'ESP resti acceso all'infinito se il destinatario è spento
      if (millis() - startTimeout > 1000) {
	data->logger->notice("nowsat Transaction timeout exceded!");
	rc=false;
	break;
      }
    }
  } else {
    data->logger->error("nowsat Error queueing the data");
    error_count++;
    state = STATE_SAT_NONE;
  }

  return rc;  
}
