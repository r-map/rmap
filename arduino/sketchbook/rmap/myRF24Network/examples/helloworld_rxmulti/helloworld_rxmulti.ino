/*
 Copyright (C) 2012 James Coliz, Jr. <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

/**
 * Simplest possible example of using RF24Network wuth multipackets and RSA
 *
 * RECEIVER NODE
 * Listens for messages from the transmitter and prints them out.
 */
 
#include <AESLib.h>
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
//const uint16_t other_node = 0;

RF24NetworkHeader header;
// payload_t payload;

char buffer[200];

// we need fundamental FILE definitions and printf declarations
#include <stdio.h>

// create a FILE structure to reference our UART output function
static FILE uartout = {0} ;

// create a output function
// This works because Serial.write, although of
// type virtual, already exists.
static int uart_putchar (char c, FILE *stream)
{
  Serial.write(c) ;
  return 0 ;
}


void setup()
{
  Serial.begin(9600);
  Serial.println("RF24Network/examples/helloworld_rx/");

// fill in the UART file descriptor with pointer to writer.
  fdev_setup_stream (&uartout, uart_putchar, NULL, _FDEV_SETUP_WRITE);

  // The uart is the standard output device STDOUT.
  stdout = &uartout ;

  delay(500);

  SPI.begin();

  radio.begin();
  //radio.printDetails();
  network.begin(/*channel*/ 93, /*node address*/ this_node,key,iv);
  //network.begin(/*channel*/ 93, /*node address*/ this_node,NULL,NULL);

  radio.setRetries(1,15);
  network.txTimeout=500;
//  radio.maskIRQ(1,1,0);
  radio.powerUp();
//  pinMode (interupin, INPUT);

}

void loop()
{

//  Serial.println("sleep");
//  delay(10);
//  network.sleep(interu,LOW,SLEEP_MODE_PWR_DOWN);

  // Pump the network regularly
  network.update();
  // Is there anything ready for us?
  //while ( network.available() ){
      
    // If so, grab it and print it out
    size_t size = network.readmulti(header,buffer,sizeof(buffer));
    if (size >0){
      Serial.print("received bytes:");
      Serial.println(size);
      Serial.println(header.toString());
      buffer[size-1]='\0';
      Serial.print(" message: ");
      Serial.println(buffer);
    } else {
      Serial.println("receive error");
    }

    //network.update();

  //}

  //delay(200);

}
