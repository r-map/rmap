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

void setup()
{

  Serial.begin(9600);
  Serial.println("SIM800 Shield testing.");

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
  Serial.println("connected");
  if (!s800.getIMEI(imeicode)){
    Serial.println(F("#GSM ERROR getting IMEI"));
  }else{
    Serial.print("IMEI: ");
    Serial.println(imeicode);
  }
}

void loop()
{
  if (!s800.getIMEI(imeicode)){
    Serial.println(F("#GSM ERROR getting IMEI"));
  }else{
    Serial.print("IMEI: ");
    Serial.println(imeicode);
  }
  delay(1000); 
}

