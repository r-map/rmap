/*
 Copyright (C) 2012 James Coliz, Jr. <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

/**
 * Simplest possible example of using RF24Network 
 *
 * TRANSMITTER NODE
 * Every 1 seconds, send a payload to the receiver node.

hardwre connections:
rf24 pin   arduino mega2560
1          gnd
2          vcc 3.3
3          9
4          10
5          52
6          51
7          50
8          18
*/

#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

uint8_t key[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
uint8_t  iv[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

// nRF24L01(+) radio attached using Getting Started board 
RF24 radio(9,10);

// Network uses that radio
RF24Network network(radio);

// Address of our node
const uint16_t this_node = 0;

// Address of the other node
const uint16_t other_node = 1;

// How often to send 'hello world to the other unit
const unsigned long interval = 5000; //ms

// When did we last send?
unsigned long last_sent;

// buffer
char buffer[200];
char bufferr[200];


void setup(void)
{
  Serial.begin(9600);
  Serial.println("RF24Network/examples/helloworld_tx/");
 
  SPI.begin();
  radio.begin();
  //network.begin(/*channel*/ 93, /*node address*/ this_node, key, iv);
  network.begin(/*channel*/ 93, /*node address*/ this_node, NULL,NULL);

  radio.setRetries(1,15);
  network.txTimeout=500;

}


void loop(void)
{
  // Pump the network regularly
  network.update();

  // If it's time to send a message, send it!
  unsigned long now = millis();
  if ( now - last_sent >= interval  )
  {
    last_sent = now;

    strcpy (buffer,"abcdefghilmnopqrstuvzABCDEFGHILMNOPQRSTUVZ0123456789");

    Serial.println("Sending...");
    RF24NetworkHeader header( other_node,0);

    bool ok = network.writemulti(header,buffer,strlen(buffer)+1);

    strcpy (bufferr,"**************************************************************************************************************");

    if (ok){
      Serial.println("ok.");
      Serial.println("Receiving...");
      size_t size = network.readmulti(header,bufferr,sizeof(bufferr));
      if (size >0){
   	Serial.print("message: "); Serial.println(bufferr);
      }else 
   	Serial.println("Error");
    } else
      Serial.println("failed.");
  }
}
