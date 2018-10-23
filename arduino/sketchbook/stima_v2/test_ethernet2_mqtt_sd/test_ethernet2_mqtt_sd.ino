#include <SPI.h>
#include <Ethernet2.h>
#include <PubSubClient.h>
#include <SdFat.h>
#include <avr/wdt.h>

#define SDCARD_CHIP_SELECT_PIN  (7)
#define W5500_CHIP_SELECT_PIN   (8)

byte mac[]    = {  0xE2, 0x21, 0xB6, 0x44, 0xEB, 0x29 };
byte server[] = { 89, 96, 234, 245 };   // rmap.cc
//byte server[] = { 92, 223, 165, 53 }; // dititeco.it
//byte server[] = { 198, 41, 30, 241 }; // server di prova eclipse

// decommentare con DHCP OFF
//byte ip[] = { 172, 20, 1, 133 };
//byte netmask[] = { 255, 255, 240, 0 };
//byte gateway[] = { 172, 20, 1, 1 };
//byte dnss[] = { 172, 20, 1, 100 };

EthernetClient ethClient;
PubSubClient client(server, 1883, ethClient);

SdFat sdcard;
File dataFile;

unsigned long start= millis();

void setup() {
  pinMode(SDCARD_CHIP_SELECT_PIN, OUTPUT);
  digitalWrite(SDCARD_CHIP_SELECT_PIN, HIGH);
  Ethernet.w5500_cspin = 8;

  // enable watchdog with timeout to 8s
  wdt_enable(WDTO_8S);
  
  Serial.begin(115200);
  Serial.println("Start...");
  SPI.begin();

  // DHCP ON
  while (Ethernet.begin(mac) == 0){
    Serial.print("dhcp failed");
    Serial.println(millis()-start);
    start= millis();
    wdt_reset();
  }

  while (!sdcard.begin(SDCARD_CHIP_SELECT_PIN)) {
    Serial.println("Card failed, or not present");
    start= millis();
    wdt_reset();
  }
  Serial.println("card initialized.");

  do {
    dataFile = sdcard.open("file_log.txt", FILE_WRITE);
    if (!dataFile) {
      Serial.println("error opening file_log.txt");
      start= millis();
      wdt_reset();
    }
  }
  while (!dataFile);

  // DHCP OFF
  //Ethernet.begin(mac, ip, dnss, gateway, netmask);

  Serial.println("Ethernet ok...");

  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("My netmask address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    Serial.print(Ethernet.subnetMask()[thisByte], DEC);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("My gateway address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    Serial.print(Ethernet.gatewayIP()[thisByte], DEC);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("My dns address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    Serial.print(Ethernet.dnsServerIP()[thisByte], DEC);
    Serial.print(".");
  }
  Serial.println();

  delay(2000);
  Serial.println("end setup");
  wdt_reset();
}

// Connect TCP
void loopnull() {
  // Connessione TCP
  w5500.setRetransmissionTime(4000); //each unit is 100us, so 0x07D0 (decimal 2000) means 200ms.
  w5500.setRetransmissionCount(3);
  if (ethClient.connect(server, 1883)) {
    Serial.println("connected");
    wdt_reset();
    ethClient.stop();
    wdt_reset();
  }
  else Serial.println("not connected");
  wdt_reset();
  delay(4000);
  wdt_reset();
}

// Connect MQTT
void loop() {
  Serial.println("Loop");
  if (!client.connected()) {
    wdt_reset();
    Serial.println("try to connect");
    Serial.println(millis()-start);
    start= millis();
    w5500.setRetransmissionTime(4000); //each unit is 100us, so 0x07D0 (decimal 2000) means 200ms.
    w5500.setRetransmissionCount(3);
    if (client.connect("testboard")) {
      Serial.println("connected");
      Serial.println(millis()-start);
      start= millis();
      wdt_reset();
      
      while (client.publish("test/prova","hello world")) {
        wdt_reset();
        
        dataFile.println("Writing in sdcard....");
        dataFile.flush();
        
        Serial.println("published");
        Serial.println(millis()-start);
        start= millis();
        delay(2000);
        wdt_reset();
        client.loop();
        wdt_reset();
      }
      Serial.println("ERRORE publish MQTT");
      wdt_reset();
      client.disconnect();  
      Serial.println("disconnected");
      wdt_reset();
      delay(2000);
      wdt_reset();
    }
  } 
}

