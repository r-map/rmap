// STM23F103C8 - Displays all traffic found on canbus port
// Initial by Thibaut Viard/Wilfredo Molina/Collin Kidder 2013-2014
// jaume clarens 2018
// Changed to fit updated CAN library, Jens F. Jensen 2020

// Required libraries
#include <STM32CAN.h>

void setup()
{
	Serial.begin(115200);
	Serial.println("Hi it's 1\n\r");
  Can1.begin(); //Use std pins (PB_8 and PB_9)
//    Can1.begin(CAN_DEFAULT_BAUD, false, false); //Use alt pins ( PA_11 and PA_12)

}

void printFrame(CAN_message_t &frame) {
   Serial.print("ID: 0x");
   Serial.print(frame.id, HEX);
   Serial.print(" Len: ");
   Serial.print(frame.dlc);
   Serial.print(" Data: 0x");
   for (int count = 0; count < frame.dlc; count++) {
       Serial.print(frame.data.bytes[count], HEX);
       Serial.print(" ");
   }
   Serial.print("\r\n");
}

void loop(){
	delay(1);
	return;
	CAN_message_t incoming;

  if (Can1.available() > 0) {
	  Can1.read(incoming);
	  printFrame(incoming);
  }
}
