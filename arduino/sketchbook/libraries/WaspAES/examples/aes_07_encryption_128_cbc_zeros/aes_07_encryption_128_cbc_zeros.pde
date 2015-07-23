/*
 *  ------Waspmote AES_07 Encryption 128 CBC ZEROS --------
 *
 *  This example shows how to encrypt plain text using AES algorithm,
 *  with 128 key size, ZEROS Padding and CBC Cipher Mode.
 *  
 *  Copyright (C) 2014 Libelium Comunicaciones Distribuidas S.L.
 *  http://www.libelium.com
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Version:                0.2
 *  Design:                 David Gascón
 *  Implementation:         Alvaro Gonzalez
 */

#include "aes.h"
#include "WaspAES.h"
 
// Define private key to encrypt message  
char password[] = "AES password"; 

// Define Initial Vector
//uint8_t IV[16] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x00,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F};
uint8_t IV[16] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

void setup(){ 
 
  // init USB port 
  Serial.begin(9600);
  Serial.println(F("AES En/De crypted example")); 
} 
 
void loop(){ 
  
  uint8_t ret;

  // original message on which the algorithm will be applied 
  char message[] = "Libelium, ma come funziona questa cosa che pare OK ma non lo è"; 
  Serial.print(F("message:")); Serial.println(message);
  AES.printMessage((uint8_t*) message, sizeof(message)); 

  // 1. Caculate block numbers of encrypted message 
  uint16_t size;
  size = AES.sizeOfBlocks(message); 
  Serial.print(F("blocked size=")); Serial.println(size);

  // 2. Declaration of variable encrypted message 
  uint8_t encrypted_message[size]; 

  // 3. Calculate encrypted message with CBC cipher mode and ZEROS5 padding. 
  ret = AES.encrypt(128,password,message,encrypted_message,CBC,ZEROS,IV); 
  Serial.println(ret);
 
  // 4. Printing encrypted message
  Serial.println(F("encrypted"));
  AES.printMessage(encrypted_message,size); 

  //1. Calculate size of encrypted message
  uint8_t length; 
  length = sizeof(encrypted_message); 
  //2. Declarete variable to store decrypted messsage 
  uint8_t decrypted_message[length+1]; 
 
  //3. Declarate variable to storre original size of  
  //   decrypted message. 
  uint16_t original_size[1]; 

  ret = AES.decrypt(128,password,encrypted_message,length,decrypted_message,original_size,CBC,ZEROS,IV); 
  Serial.println(ret);

  //5. Print on serial output decrypted message  
  Serial.println(F("decrypted")); 
  Serial.println(original_size[0],DEC);
  // Serial.print("\""); 
  // for (uint8_t h=0; h< 8;h++){ 
  //     Serial.print(decrypted_message[h]); 
  // } 
  // Serial.println("\""); 

  decrypted_message[original_size[0]]=0x00;
  AES.printMessage(decrypted_message, original_size[0]); 
  Serial.print(F("message decrypted:")); Serial.println((char*)decrypted_message);

  //6. Free memory 
  //free(decrypted_message); 
 
  delay(5000);
 
}
