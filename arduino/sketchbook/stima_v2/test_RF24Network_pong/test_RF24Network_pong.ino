/*
 Copyright (C) 2012 James Coliz, Jr. <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

/**
 * Simplest possible example of using RF24Network,
 *
 * RECEIVER NODE
 * Listens for messages from the transmitter and prints them out.
 */

#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <avr/sleep.h>

/*
Board	          int.0	    int.1	int.2	int.3	int.4	int.5
Uno, Ethernet	      2 	3	 	 	 	 
Mega2560	      	        	   21	   20	   19	   18
*/
const int interu = 1;
const int interupin = 3;

uint8_t key[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
uint8_t  iv[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

// nRF24L01(+) radio attached using Getting Started board
//Parameters
//    [in]	chipEnablePin	the Arduino pin to use to enable the chip for4 transmit/receive
//    [in]	chipSelectPin	the Arduino pin number of the output to use to select the NRF24 before accessing it 
RF24 radio(9,10);

// Network uses that radio
RF24Network network(radio);

// Address of our node
const uint16_t this_node = 1;

// Address of the other node
const uint16_t other_node = 0;

RF24NetworkHeader header;
// payload_t payload;

char buffer[200];

void setup()
{
  Serial.begin(9600);
  Serial.println("RF24Network/examples/helloworld_rx/");
  delay(500);

  SPI.begin();

  radio.begin();
  //radio.printDetails();
  //network.begin(/*channel*/ 93, /*node address*/ this_node,key,iv);
  network.begin(/*channel*/ 93, /*node address*/ this_node,NULL,NULL);

  radio.setRetries(1,15);
  network.txTimeout=500;
  radio.maskIRQ(1,1,0);
  radio.powerUp();
  //sleep.pwrDownMode(); //sets the Arduino into power Down Mode sleep, the most power saving, all systems are powered down except the watch dog timer and external reset
  pinMode (interupin, INPUT);

}

void loop()
{

  Serial.println("sleep");
  delay(50);
  network.sleep(interu,LOW,SLEEP_MODE_PWR_DOWN);

  // Pump the network regularly
  network.update();
  // Is there anything ready for us?
  while ( network.available() ){
      
  //   // If so, grab it and print it out

    strcpy(buffer,"***************************************************************************************");

    size_t size = network.readmulti(header,buffer,sizeof(buffer));
    if (size >0){
      //Serial.print("received bytes:");
      //Serial.println(size);
      //Serial.println(header.toString());
      buffer[size-1]='\0';
      Serial.print(" message: ");
      Serial.println(buffer);

      strcpy(buffer,"errore\0");
      strcpy(buffer,"{\"jsonrpc\": \"2.0\",\"result\":{\"B12101\":29802},\"id\":0}");

      Serial.print("response: ");
      Serial.println(buffer);
      RF24NetworkHeader sendheader(other_node,0);

      bool ok= network.writemulti(sendheader,buffer,strlen(buffer)+1);
      if (!ok){
	Serial.println("error");
      }
      delay(200);
    } else {
      Serial.println("receive error");      
    }

    network.update();

  }
}
