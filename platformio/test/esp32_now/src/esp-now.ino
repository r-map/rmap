#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

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

// REPLACE WITH THE MAC Address of your receiver 
//uint8_t broadcastAddress[] = {0x18,0x8b,0x0e,0x04,0x2c,0x0c};
//uint8_t broadcastAddress[] = {0x70,0x04,0x1d,0x22,0x73,0xe8};
uint8_t broadcastAddress[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

bool ioaccoppiato = false;
bool tuaccoppiato = false;
uint8_t serverMac[6];

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
  int type;
  float temp;
  float hum;
  float pres;
} struct_message;

// Callback when data is sent
void OnDataSent(const uint8_t *mac, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  Serial.print("destination MAC:");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X:", mac[i]);
  }
  Serial.println();
}

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  // Create a struct_message to hold incoming sensor readings
  struct_message incomingReadings;
  
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  Serial.println();
  Serial.print("Bytes received: ");
  Serial.println(len);
  // Controlla se è una richiesta di Pairing
  if (incomingReadings.type == 0) {
    Serial.print("Richiesta di pairing ricevuta da MAC: ");
    for (int i = 0; i < 6; i++) {
      Serial.printf("%02X:", mac[i]);
    }
    Serial.println();
        
    // Risponde al trasmettitore per confermare il pairing
    // Create a struct_message called Readings to hold sensor readings
    struct_message outgoingReadings;
    outgoingReadings.type=1;
    esp_now_send(broadcastAddress, (uint8_t *) &outgoingReadings, sizeof(outgoingReadings));
    tuaccoppiato=true;

  } else if (incomingReadings.type == 1 ) {
        
    // Aggiunge il server come peer specifico
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, mac, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    
    if (esp_now_add_peer(&peerInfo) == ESP_OK) {
      Serial.println("Server registrato come peer fisso.");      
      Serial.println("TU Accoppiamento riuscito!");
      memcpy(serverMac, mac, 6); // Salva il MAC reale del server
      ioaccoppiato=true;      
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

  WiFi.STA.begin();
  Serial.print("[DEFAULT] ESP32 Board MAC Address: ");
  readMacAddress();
  memcpy(serverMac, broadcastAddress, 6); // initialize MAC reale del server
  
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(esp_now_send_cb_t(OnDataSent));
  
  // Register peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}
 
void loop() {

  // Create a struct_message called Readings to hold sensor readings
  struct_message outgoingReadings;
  
  if (ioaccoppiato and tuaccoppiato){
    //Rimuove il peer broadcast generico
    esp_now_del_peer(broadcastAddress);

    outgoingReadings.type=99;
    // Set values to send
    outgoingReadings.temp = 273;
    outgoingReadings.hum = 50;
    outgoingReadings.pres = 1013;

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(serverMac, (uint8_t *) &outgoingReadings, sizeof(outgoingReadings));
    
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    }
    else {
      Serial.println("Error sending the data");
    }
  }else{
    outgoingReadings.type=0;
    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &outgoingReadings, sizeof(outgoingReadings));
    
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    }
    else {
      Serial.println("Error sending the data");
    }
  }

  delay(5000);
}
