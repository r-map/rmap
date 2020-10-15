// STM23F103C8 - Displays all traffic found on canbus port
// Initial by Thibaut Viard/Wilfredo Molina/Collin Kidder 2013-2014
// jaume clarens 2018
// Changed to fit updated CAN library, Jens F. Jensen 2020

// Required libraries
#include <STM32CAN.h>

HardwareSerial Serial2(PA3, PA2);  //uart2

void setup()
{
  Serial2.begin(115200);
  Serial2.println("Hi it's 1");
  delay(3000);
  //Can1.begin(); //Use std pins (PB_8 and PB_9)
  Can1.begin(CAN_BPS_250K, false, false); //Use alt pins ( PA_11 and PA_12)
}

void printFrame(CAN_message_t &frame) {
   Serial2.print("ID: 0x");
   Serial2.print(frame.id, HEX);
   Serial2.print(" Len: ");
   Serial2.print(frame.dlc);
   Serial2.print(" Data: 0x");
   for (int count = 0; count < frame.dlc; count++) {
     Serial2.print(frame.data.bytes[count], HEX);
     Serial2.print(" ");
   }
   Serial2.print("\r\n");
}

CAN_message_t incoming;

void loop(){
  
  if (Can1.available() > 0) {
    Can1.read(incoming);
    printFrame(incoming);
  }
}
