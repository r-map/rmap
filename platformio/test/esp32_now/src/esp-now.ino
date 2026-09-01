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

#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
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

RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR bool ioaccoppiato = false;
RTC_DATA_ATTR bool tuaccoppiato = false;
RTC_DATA_ATTR uint8_t serverMac[6];
RTC_DATA_ATTR uint8_t channel = 0;
RTC_DATA_ATTR uint8_t error_count = 0;

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
  int type;
  float temp;
  float hum;
  float pres;
} struct_message;

/*
Method to print the reason by which ESP32
has been awaken from sleep
*/
void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:     Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1:     Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER:    Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD: Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP:      Serial.println("Wakeup caused by ULP program"); break;
    default:                        Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  }
}


void readMacAddress(){
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
                  baseMac[0], baseMac[1], baseMac[2],
                  baseMac[3], baseMac[4], baseMac[5]);
    Serial.println();
  } else {
    Serial.println("Failed to read MAC address");
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
    Serial.println("peer broadcast già registrato");
  }else{    
    // Add peer        
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
      Serial.println("Failed to add broadcast peer");
    }
  }
}


// Callback when data is sent
void OnDataSent(const  uint8_t *des_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");

  if (status != ESP_NOW_SEND_SUCCESS) error_count++;
  Serial.print("destination MAC:");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X:", des_addr[i]);
  }
  Serial.println();  
}

// Callback when data is received
void OnDataRecv(const esp_now_recv_info_t *esp_now_info, const uint8_t *incomingData, int len) {
  // Create a struct_message to hold incoming sensor readings
  struct_message incomingReadings;
  Serial.print("Pacchetto ricevuto da MAC: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X:", esp_now_info->src_addr[i]);
  }
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  Serial.print("Bytes received: ");
  Serial.println(len);
  // Controlla se è una richiesta di Pairing
  if (incomingReadings.type == 0) {
    Serial.print("Richiesta di pairing ricevuta da MAC: ");
    for (int i = 0; i < 6; i++) {
      Serial.printf("%02X:", esp_now_info->src_addr[i]);
    }
    Serial.println();
        
    // Risponde al trasmettitore per confermare il pairing
    // Create a struct_message called Readings to hold sensor readings
    struct_message outgoingMessage;
    outgoingMessage.type=1;
    esp_now_send(broadcastAddress, (uint8_t *) &outgoingMessage, sizeof(outgoingMessage));
    tuaccoppiato=true;

  } else if (incomingReadings.type == 1 ) {

    Serial.println("risposta al broadcast ricevuta");
    if (esp_now_is_peer_exist(esp_now_info->src_addr)){
      ioaccoppiato=true;
      Serial.println("peer già registrato");
    }else{    
      // Aggiunge il server come peer specifico
      esp_now_peer_info_t peerInfo = {};
      memcpy(peerInfo.peer_addr, esp_now_info->src_addr, 6);
      peerInfo.channel = 0;
      //peerInfo.ifidx = WIFI_IF_STA;  // Interfaccia usata (Station o AP)
      peerInfo.encrypt = true;         // enable with pioarduino only! tasmota configurated with no encryption
      memcpy(peerInfo.lmk, mio_lmk, 16);
      
      if (esp_now_add_peer(&peerInfo) == ESP_OK) {
	Serial.println("Server registrato come peer fisso.");      
	Serial.println("TU Accoppiamento riuscito!");
	memcpy(serverMac, esp_now_info->src_addr, 6); // Salva il MAC reale del server
	ioaccoppiato=true;
      }else{
	Serial.println("Error adding peer");
      }
    }
  } else if (incomingReadings.type == 99) {

    Serial.println("Values received:");
    Serial.println(incomingReadings.temp);
    Serial.println(incomingReadings.hum);
    Serial.println(incomingReadings.pres);

  }
}

void setup() {

  // Init Serial Monitor
  Serial.begin(115200);
  delay(5000);

  //Increment boot number and print it every reboot
  ++bootCount;
  Serial.print("Boot number: ");
  Serial.println(bootCount);

  //Print the wakeup reason for ESP32
  print_wakeup_reason();


  // Change ESP32 Mac Address
  //  esp_err_t err = esp_wifi_set_mac(WIFI_IF_STA, &newMACAddress[0]);
  //if (err == ESP_OK) {
  //  Serial.println("Success changing Mac Address");
  //}
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  
  // ESP-Now Range Test: Real-World Results for ESP32 Devices
  // https://youtu.be/oz0a7Ur7nko?si=aUMBJ4SpeXTSMMPg
  esp_err_t err = esp_wifi_set_protocol(WIFI_IF_STA,WIFI_PROTOCOL_LR);
  if (err == ESP_OK) {
    Serial.println("Protocol successfully restricted to Long Range (LR) Mode successfully enabled!");
  } else {
    Serial.printf("Failed to set protocol. Error code: %d\n", err);
  }
  
  WiFi.STA.begin();
  Serial.print("[DEFAULT] ESP32 Board MAC Address: ");
  readMacAddress();
  
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Imposta la PMK globale
  // Se questa funzione fallisce o non viene chiamata, ESP-NOW userà una PMK di default della mesh,
  // compromettendo la sicurezza dell'intero ecosistema.
  if (esp_now_set_pmk(mia_pmk) == ESP_OK) {
    Serial.println("PMK impostata con successo!");
  } else {
    Serial.println("Errore nell'impostazione della PMK");
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

  Serial.println("Ricevitore ESP-NOW pronto in modalità Light Sleep Automatico.");

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
  
  if (bootCount > 0 and ioaccoppiato and tuaccoppiato){
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
    // Aggiunge il server come peer specifico
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, serverMac, 6);
    peerInfo.channel = 0;
    //peerInfo.ifidx = WIFI_IF_STA;  // Interfaccia usata (Station o AP)
    peerInfo.encrypt = true;         // enable with pioarduino only! tasmota configurated with no encryption
    memcpy(peerInfo.lmk, mio_lmk, 16);
    
    if (esp_now_add_peer(&peerInfo) == ESP_OK) {
      Serial.println("boot Server registrato come peer fisso.");      
      Serial.println("boot TU Accoppiamento riuscito!");
    }else{
      Serial.println("boot Error adding peer");
    }
  }

  /*
  First we configure the wake up source
  We set our ESP32 to wake up every 5 seconds
  */
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");

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
  //Serial.println("Configured all RTC Peripherals to be powered down in sleep");
  
}
 
void loop() {

  // Create a struct_message called Readings to hold sensor readings
  struct_message outgoingMessage;
  
  if (ioaccoppiato and tuaccoppiato){
    //Rimuove il peer broadcast generico
    esp_now_del_peer(broadcastAddress);

    outgoingMessage.type=99;
    // Set values to send
    outgoingMessage.temp = random(250,300);
    outgoingMessage.hum = random(1,100);
    outgoingMessage.pres = random(990,1030);

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(serverMac, (uint8_t *) &outgoingMessage, sizeof(outgoingMessage));
    
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    }
    else {
      Serial.println("Error sending the data");
      uint8_t channel = 0;
      error_count++;
    }

    if (error_count > 5){
      ioaccoppiato=false;
      tuaccoppiato=false;
      add_broadcast_peer();
      Serial.println("peer do not respond: restart pairing");
      error_count=0;
    }else{

      /*
	Now that we have setup a wake cause and if needed setup the
	peripherals state in deep sleep, we can now start going to
	deep sleep.
	In the case that no wake up sources were provided but deep
	sleep was started, it will sleep forever unless hardware
	reset occurs.
      */
      Serial.println("Going to sleep now");
      Serial.flush();
      esp_deep_sleep_start();
      Serial.println("This will never be printed");
    }
  }else{

    add_broadcast_peer();
    outgoingMessage.type=0;
    Serial.println("Send broadcast message");
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &outgoingMessage, sizeof(outgoingMessage));
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    }
    else {
      Serial.println("Error sending the data");
    }
    delay(500);
  }

  if (!ioaccoppiato and !tuaccoppiato){
    channel=random(1,14) ;
    Serial.print("channel: ");
    Serial.println(channel);
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  }
}
