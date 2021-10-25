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
//#define MQTT_DEBUG 1
#define MQTTCLIENT_QOS2 1

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <WifiIPStack.h>
#include <Countdown.h>
#include <MQTTClient.h>

volatile int arrivedcount = 0;

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

WiFiClient c; // replace by a YunClient if running on a Yun
IPStack ipstack(c);
MQTT::Client<IPStack, Countdown, 200, 1> client = MQTT::Client<IPStack, Countdown, 200, 1>(ipstack);

const char* topic = "test/MQTTClient/WifiHello";

void connect()
{
  char hostname[] = "rmap.cc";
  int port = 1883;

  Serial.print("Connecting to ");
  Serial.print(hostname);
  Serial.print(":");
  Serial.println(port);
 
  int rc = ipstack.connect(hostname, port);
  if (rc != 1)
  {
    Serial.print("rc from TCP connect is ");
    Serial.println(rc);
  }
 
  Serial.println("MQTT connecting");
  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;       
  data.MQTTVersion = 3;
  data.will.topicName.cstring = "test/maint/test/test1/test2/test3/test4/test5/test6/test7/test8/test9/test10";
  data.will.message.cstring = "error";
  data.will.retained = true;
  data.will.qos = MQTT::QOS1;
  data.willFlag = true;
  data.keepAliveInterval=5;
  
  data.clientID.cstring = (char*)"arduino-sample";
  rc = client.connect(data);
  if (rc != 0)
  {
    Serial.print("rc from MQTT connect is ");
    Serial.println(rc);
  }
  Serial.println("MQTT connected");
  
  rc = client.subscribe(topic, MQTT::QOS2, messageArrived);   
  if (rc != 0)
  {
    Serial.print("rc from MQTT subscribe is ");
    Serial.println(rc);
  }
  Serial.println("MQTT subscribed");
}

void setup()
{
  Serial.begin(9600);
  Serial.println("MQTT Hello example");
  WiFi.begin("ssed_tosetup", "password_tosetup");

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
 
  connect();
}

MQTT::Message message;

void loop()
{ 
  Serial.println("Loop");
  if (!client.isConnected())
    connect();
    
  arrivedcount = 0;

  // Send and receive QoS 0 message
  char buf[100];
  strcpy(buf, "Hello World! QoS 0 message");
  message.qos = MQTT::QOS0;
  message.retained = false;
  message.dup = false;
  message.payload = (void*)buf;
  message.payloadlen = strlen(buf)+1;
  int rc = client.publish(topic, message);
  if (rc != 0)
  {
    Serial.print("rc from MQTT pubblish QoS 0 is ");
    Serial.println(rc);
    arrivedcount ++;
  }

  while (arrivedcount == 0)
  {
    Serial.println("Waiting for QoS 0 message");
    client.yield(1000);
  }
  
  // Send and receive QoS 1 message
  strcpy(buf, "Hello World! QoS 1 message");
  message.qos = MQTT::QOS1;
  message.payloadlen = strlen(buf)+1;
  rc = client.publish(topic, message);
  if (rc != 0)
  {
    Serial.print("rc from MQTT pubblish QoS 1 is ");
    Serial.println(rc);
    arrivedcount ++;
  }

  while (arrivedcount == 1)
  {
    Serial.println("Waiting for QoS 1 message");
    client.yield(1000);
  }

  // Send and receive QoS 2 message
  strcpy(buf, "Hello World! QoS 2 message");
  message.qos = MQTT::QOS2;
  message.payloadlen = strlen(buf)+1;
  rc = client.publish(topic, message);
  if (rc != 0)
  {
    Serial.print("rc from MQTT pubblish QoS 2 is ");
    Serial.println(rc);
    arrivedcount ++;
  }
  while (arrivedcount == 2)
  {
    Serial.println("Waiting for QoS 2 message");
    client.yield(1000);  
  }
  delay(2000);
}
