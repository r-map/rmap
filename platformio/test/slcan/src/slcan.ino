// LAWICEL slcan 2.0
// http://www.can232.com/docs/canusb_manual.pdf

/*
Copyright (C) 2015  Paolo Paruno <p.patruno@iperbole.bologna.it>
authors:
Paolo Patruno <p.patruno@iperbole.bologna.it>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Freeg Software Foundation; either version 2 of 
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

int baudrate = 250000;
bool connected=false;

// Required libraries
#include <STM32CAN.h>

//HardwareSerial Serial2(PA3, PA2);  //uart2

uint8_t can_speed(char speed){

  switch (speed) {
  case '0':
    baudrate = 10000;
    break;
  case '1':
    baudrate = 20000;
    break;
  case '2':
    baudrate = 50000;
    break;
  case '3':
    baudrate = 100000;
    break;
  case '4':
    baudrate = 125000;
    break;
  case '5':
    baudrate = 250000;
    break;
  case '6':
    baudrate = 500000;
    break;
  case '7':
    baudrate = 800000;
    break;
  case '8':
    baudrate = 1000000;
    break;
  default:
    return -1;
    break;
  }
  return 0;
}


void command (void){

  char c;
  #define BUFFERLEN 100
  char buffer[BUFFERLEN];
  char* data;
  int ide=1;
  int dlc;
  unsigned int id;
  bool send = true;
  bool rtr = false;
  int status = 0;
  char speed='9';

  size_t size=Serial.readBytesUntil({13}, buffer, BUFFERLEN);
  if (size>0){
    buffer[size]='\0';
    sscanf (buffer,"%1s",&c);

    switch (c) {
    case 'T':
      //Serial2.println("command T");
      if (sscanf (buffer,"%1s%8x%1i",&c,&id,&dlc) ==3){
	data=&buffer[10];
      } else status=1;
      break;
    case 't':
      ide = 0;
      //Serial2.println("command t");
      if (sscanf (buffer,"%1s%3x%1i",&c,&id,&dlc) ==3){
	data=&buffer[5];
      } else status=1;
      break;
    case 'R':
      //Serial2.println("command R");
      if (sscanf (buffer,"%1s%8x%1i",&c,&id,&dlc) ==3){
	rtr = true;
	data=&buffer[10];
      } else status=1;
      break;
    case 'r':
      ide = 0;
      //Serial2.println("command r");
      if (sscanf (buffer,"%1s%3x%1i",&c,&id,&dlc) ==3){
	data=&buffer[5];
      } else status=1;
      break;
    case 'S':
      //Serial2.println("command S");
      if (sscanf (buffer,"%1s%1s",&c,&speed) ==2){
	status = can_speed(speed);
	send = false;
      } else status=1;
      break;
    case 'v':
      //Serial2.println("command v");
      send = false;
      break;
    case 'V':
      //Serial2.println("command V");
      send = false;
      break;
    case 'C':
      //Serial2.println("command C");
      send = false;
      connected=false;
      break;
    case 'O':
      //Serial2.println("command O");
      //Can1.begin(); //Use std pins (PB_8 and PB_9)
      Can1.begin(baudrate, false, false); //Use alt pins ( PA_11 and PA_12)
      send = false;
      connected=true;
      break;
    default:
      send = false;
      status = 1;
    }

    if(status == 0){
      if (send){
	if (connected) {
	  if(dlc <= 8) {
	    CAN_message_t frame;
	    frame.ide=ide;
	    frame.id=id;
	    frame.dlc=dlc;
	    frame.rtr=rtr;
	    //Serial2.print("ide:");
	    //Serial2.println(frame.ide);
	    //Serial2.print("id:");
	    //Serial2.println(frame.id);
	    //Serial2.print("rtr:");
	    //Serial2.println(frame.rtr);
	    //Serial2.print("dlc:");
	    //Serial2.println(frame.dlc);
	    //Serial2.print("data:");
	    //Serial2.println(data);
			
	    for (int count = 0; count < frame.dlc; count++) {
	      if (status ==0){
		if (sscanf(data,"%2x",&frame.data.bytes[count]) == 1){
		  ////Serial2.print("data HEX:");
		  ////Serial2.println(frame.data.bytes[count],HEX);
		  data++;
		  data++;
		} else status=1;
	      }
	    }
	    
	    //for (int count = 0; count < frame.dlc; count++) {
	    //  //Serial2.println(frame.data.bytes[count]);
	    //}

	    if (status == 0){
	      if (Can1.write(frame)) {
		status =0;
	      }else{
		status =1;
	      }
	    }
	  }
	  else
	    status = 1;
	}
	else
	  status = 1;
      }
    }
  }

  if (status == 0){
    Serial.print("\r");
    //Serial2.println("OK");
  }else{	  
    Serial.print("\a");
    //Serial2.println("KO");
  }    
}



void data(CAN_message_t &frame) {

   char c;
   char buff[100];
   if (frame.ide) {
     if (frame.rtr)
       c = 'R';
     else
       c = 'T';
  
     snprintf(buff, sizeof(buff), "%c%08X%1d",c,frame.id,frame.dlc);

   } else {
     if (frame.rtr)
       c = 'r';
     else
       c = 't';

     snprintf(buff, sizeof(buff), "%c%03X%1d",c,frame.id,frame.dlc);
   }

   Serial.print(buff);
   for (int count = 0; count < frame.dlc; count++) {
     snprintf(buff, sizeof(buff), "%02X",frame.data.bytes[count]);
     Serial.print(buff);
     //Serial2.print(buff);
   }
   
   Serial.print("\r");
   //Serial2.println("");
   
}


void setup()
{
  Serial.begin(115200);
  //Serial2.begin(115200);
  //Serial2.println("Slcan started");
}


void loop(){
  
  if (Can1.available() > 0) {
    CAN_message_t incoming;
    Can1.read(incoming);
    data(incoming);
  }

  if (Serial.available() > 0) {
    command();
  }

}
