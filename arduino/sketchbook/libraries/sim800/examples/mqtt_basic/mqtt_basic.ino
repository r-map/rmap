/*
 Basic MQTT example 
 
  - connects to an MQTT server
  - publishes "hello world" to the topic "test/out"
  - subscribes to the topic "test/in"
*/

#define GSMAPN "ibox.tim.it"
#define GSMUSER ""
#define GSMPASSWORD ""

#include <Time.h>
#include <sim800Client.h>
#include <PubSubClient.h>
#include <TimeAlarms.h>

sim800Client s800;
char imeicode[16];

// Update these with values suitable for your network.
//byte server[] = { 192, 168, 1, 199 };
char server[] = "rmap.cc";


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

  Serial.begin(9600);
  Serial.println("SIM800 Shield testing.");

  //Ethernet.begin(mac, ip);

  for (int i=0; i<10; i++){
    delay(5000);
    Serial.println("try to init sim800");

#ifdef HARDWARESERIAL
    if (s800.init( 7, 6)) break;
#else
    if (s800.init(&Serial1 , 7, 6)) break;
#endif

  }

  Serial.println("try to setup sim800");
  s800.setup();
  s800.stop();
  s800.TCPstop();
  s800.getIMEI(imeicode);
  Serial.print("IMEI: ");
  Serial.println(imeicode);


  while (!s800.TCPstart(GSMAPN,GSMUSER,GSMPASSWORD)) {
    Serial.println("TCPstart failed");
    s800.TCPstop();
    delay(1000);
  }
  Serial.println("TCPstart started");

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

