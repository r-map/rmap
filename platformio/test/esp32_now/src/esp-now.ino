#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

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

bool ioaccoppiato = false;
bool tuaccoppiato = false;
uint8_t serverMac[6];
uint8_t channel = 0;
uint8_t error_count = 0;

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
  int type;
  float temp;
  float hum;
  float pres;
} struct_message;


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
  peerInfo.ifidx = WIFI_IF_STA;  // Interfaccia usata (Station o AP)
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
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
    struct_message outgoingReadings;
    outgoingReadings.type=1;
    esp_now_send(broadcastAddress, (uint8_t *) &outgoingReadings, sizeof(outgoingReadings));
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

  // Change ESP32 Mac Address
  //  esp_err_t err = esp_wifi_set_mac(WIFI_IF_STA, &newMACAddress[0]);
  //if (err == ESP_OK) {
  //  Serial.println("Success changing Mac Address");
  //}
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
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

  add_broadcast_peer();

}
 
void loop() {

  // Create a struct_message called Readings to hold sensor readings
  struct_message outgoingReadings;
  
  if (ioaccoppiato and tuaccoppiato){
    //Rimuove il peer broadcast generico
    esp_now_del_peer(broadcastAddress);

    outgoingReadings.type=99;
    // Set values to send
    outgoingReadings.temp = random(250,300);
    outgoingReadings.hum = random(1,100);
    outgoingReadings.pres = random(990,1030);

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(serverMac, (uint8_t *) &outgoingReadings, sizeof(outgoingReadings));
    
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
    }
    
  }else{
    outgoingReadings.type=0;
    Serial.println("Send broadcast message");
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &outgoingReadings, sizeof(outgoingReadings));
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    }
    else {
      Serial.println("Error sending the data");
    }
  }

  if (!ioaccoppiato and !tuaccoppiato){
    channel=random(1,14) ;
    Serial.print("channel: ");
    Serial.println(channel);
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);

  }
  delay(500);
}
