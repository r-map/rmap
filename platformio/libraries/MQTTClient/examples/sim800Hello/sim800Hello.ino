/*******************************************************************************
 * Copyright (c) 2014, 2015 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution. 
 *
 * The Eclipse Public License is available at 
 *   http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at 
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial contribution
 *    Benjamin Cabe - adapt to IPStack, and add Yun instructions
 *    Ian Craggs - remove sprintfs to reduce sketch size
 *******************************************************************************/

#define WARN Serial.println

#define MQTTCLIENT_QOS2 1

#define GSMAPN "ibox.tim.it"
#define GSMUSER ""
#define GSMPASSWORD ""

#include <SPI.h>
//#include <Ethernet.h>
#include <sim800Client.h>
#include <Sim800IPStack.h>
#include <Countdown.h>
#include <MQTTClient.h>

int arrivedcount = 0;

void messageArrived(MQTT::MessageData& md)
{
  MQTT::Message &message = md.message;
  
  Serial.print("Message ");
  Serial.print(++arrivedcount);
  Serial.print(" arrived: qos ");
  Serial.print(message.qos);
  Serial.print(", retained ");
  Serial.print(message.retained);
  Serial.print(", dup ");
  Serial.print(message.dup);
  Serial.print(", packetid ");
  Serial.println(message.id);
  Serial.print("Payload ");
  Serial.println((char*)message.payload);
}


sim800Client s800;
IPStack ipstack(s800);
char imeicode[16];
MQTT::Client<IPStack, Countdown, 120, 2> client = MQTT::Client<IPStack, Countdown, 120, 2>(ipstack,30000);

const char* topic = "test/MQTTClient/sim800";

void connect()
{
  char hostname[] = "rmap.cc";
  int port = 1883;

  Serial.println("close modem connections");
  client.disconnect();
  ipstack.disconnect();
  s800.stop();
  s800.TCPstop();


  Serial.println("Reset modem status");
  s800.init_onceautobaud();
  if (s800.setup()){
    Serial.println("s800 setup ok");
  }else{
    Serial.println("Error s800 setup");
    return;
  }

  if (s800.getIMEI(imeicode)){
    Serial.print("IMEI: ");
    Serial.println(imeicode);
  }else{
    Serial.println("Error getting IMEI code");
  }


  if (s800.TCPstart(GSMAPN,GSMUSER,GSMPASSWORD)) {
    Serial.println("TCP started");
  }else{
    Serial.println("start TCP failed");
    return;
  }

  Serial.print("Connecting to ");
  Serial.print(hostname);
  Serial.print(":");
  Serial.println(port);
 
  int rc = ipstack.connect(hostname, port);
  if (rc != 1)
  {
    Serial.print("error rc from TCP connect is ");
    Serial.println(rc);
    return;
  }else{
    Serial.println("TCP connected");
  }

  Serial.println("MQTT connecting");
  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;       
  data.MQTTVersion = 3;
  data.clientID.cstring = imeicode;
  rc = client.connect(data);
  if (rc != 0)
  {
    Serial.print("error rc from MQTT connect is ");
    Serial.println(rc);
    return;
  }else{
    Serial.println("MQTT connected");
  }

  rc = client.subscribe(topic, MQTT::QOS2, messageArrived);   
  if (rc != 0)
  {
    Serial.print("rc from MQTT subscribe is ");
    Serial.println(rc);
  }else{
    Serial.println("MQTT subscribed");
  }
}

void setup()
{
  Serial.begin(9600);
  Serial.println("MQTT Hello example");

  for (int i=0; i<10; i++){
    delay(5000);
    Serial.println("try to init sim800");

#ifdef HARDWARESERIAL
    if (s800.init( 7, 6)) break;
#else
    if (s800.init(&Serial1 , 7, 6)) break;
#endif

  }

  connect();

}

MQTT::Message message;

void loop()
{ 
  Serial.println("Loop");
  if (!client.isConnected())
    connect();
    
  arrivedcount = 0;
  int rc;
  char buf[100];

  message.retained = false;
  message.dup = false;
  message.payload = (void*)buf;

  // Send and receive QoS 0 message
  strcpy(buf, "Hello World! QoS 0 message");
  Serial.println(buf);
  message.qos = MQTT::QOS0;
  message.payloadlen = strlen(buf)+1;
  rc = client.publish(topic, message);
  if (rc != 0)
  {
    Serial.print("rc from MQTT pubblish is ");
    Serial.println(rc);
    arrivedcount ++;
  }

  int i=0;
  while (arrivedcount < 1 && i < 10)
  {
    Serial.println("Waiting for QoS 0 message");
    client.yield(10000L);
    i++;
  }

  if (arrivedcount < 1 && i == 10){
    Serial.println("message lost, Q0S 0");
    arrivedcount ++;
  }
  

  // Send and receive QoS 1 message
  strcpy(buf, "Hello World! QoS 1 message");
  Serial.println(buf);
  message.qos = MQTT::QOS1;
  message.payloadlen = strlen(buf)+1;
  rc = client.publish(topic, message);
  if (rc != 0)
  {
    Serial.print("rc from MQTT pubblish is ");
    Serial.println(rc);
    arrivedcount ++;
  }

  while (arrivedcount < 1)
  {
    Serial.println("Waiting for QoS 1 message");
    client.yield(10000L);
  }

  // Send and receive QoS 2 message
  strcpy(buf, "Hello World! QoS 2 message");
  Serial.println(buf);
  message.qos = MQTT::QOS2;
  message.payloadlen = strlen(buf)+1;
  rc = client.publish(topic, message);
  if (rc != 0)
  {
    Serial.print("rc from MQTT pubblish is ");
    Serial.println(rc);
    arrivedcount ++;
  }

  while (arrivedcount < 3)
  {
    Serial.println("Waiting for QoS 2 message");
    client.yield(10000L);  
  }

  client.yield(10L);  

  //delay(2000);
}
