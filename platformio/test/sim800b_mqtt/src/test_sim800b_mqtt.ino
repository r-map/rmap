/*
 Basic MQTT example 
 
  - connects to an MQTT server
  - publishes "hello world" to the topic "outTopic"
  - subscribes to the topic "inTopic"
*/

#define USE_SIM_800C 
#define GSMAPN "ibox.tim.it"
#define GSMUSER ""
#define GSMPASSWORD ""

#include <Time.h>
#include <sim800bClient.h>
#include <PubSubClient.h>
#include <TimeAlarms.h>

sim800Client s800;
char imeicode[16];

// Update these with values suitable for your network.
//byte server[] = { 192, 168, 1, 199 };
char server[] = "rmap.cc";


#ifndef HARDWARESERIAL
HardwareSerial Serial1(PB_11, PB_10);
#endif

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  char mypl[48];
  Serial.println(length);
  memcpy(mypl,payload,length);
  mypl[length]=char(0);
  Serial.print("receive: ");
  Serial.print(topic);
  Serial.print("->");
  Serial.println(mypl);
}

PubSubClient client(server, 1883, callback, s800);


void pub()
{
  Serial.print("send: ");
  Serial.print("test/out");
  Serial.print("->");
  Serial.println("bye bye");
  client.publish("test/out","bye bye");
}

void setup()
{

  Serial.begin(115200);
  Serial.println("SIM800 Shield testing.");

  //Ethernet.begin(mac, ip);

  for (int i=0; i<10; i++){
    delay(5000);
    Serial.println("try to init sim800");

    // HARDWARESERIAL come fron sim800b library
    #ifdef HARDWARESERIAL
    // use this on AVR
    #ifdef USE_SIM_800L
    if (s800.init( 7, 6)) break;
    #endif

    #ifdef USE_SIM_800C 
    if (s800.init( 5, 6)) break;
    #endif

    #else
    // use this on stm32
    if (s800.init(&Serial1 , D5, D6)) break;
    #endif

  }

  Serial.println("try to setup sim800");
  s800.setup();
  s800.stop();
  s800.TCPstop();
  s800.getIMEI(imeicode);
  s800.TCPstart(GSMAPN,GSMUSER,GSMPASSWORD);

  while (!client.connect(imeicode)) {
    Serial.println("connect failed");
    delay(1000);
  }

  Serial.println("connected");
  client.publish("test/out","hello world");
  client.subscribe("test/in");
  Alarm.timerRepeat(5, pub);             // timer

}

void loop()
{

    client.loop();
    Alarm.delay(100); 

}

