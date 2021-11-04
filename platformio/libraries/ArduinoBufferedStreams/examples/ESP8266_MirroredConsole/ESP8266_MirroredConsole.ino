/* 
  MirroredConsole - Example Serial + Telnet console debug for esp8266
  using LoopbackStream library.
  
  Author: Guillaume Sartre
  
  This file is part of the LoopbackStream library for Arduino environment.
 
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include <ESP8266WiFi.h>
#include "StreamPrint.h"
#include <LoopbackStream.h>

// buffer size
#define DEBUGBUFFER 256

const char* ssid = "SSID";
const char* password = "pass";

WiFiServer server(23);
WiFiClient serverClient;


const char* ex_text = "This text goes to ";
const char* ex_text2 = "Serial";
const char* ex_text3 = "Console";

LoopbackStream buffer(DEBUGBUFFER); // Loopback buffer for Serial & Telnet console debug output 
void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial << "\nConnecting to " << ssid << endl;
  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED && i++ < 20) delay(500);
  if(i == 21){
    Serial << "Could not connect to" <<  ssid << endl;
    while(1) delay(500);
  }

  // Start the telnet server
  server.begin();
  server.setNoDelay(true);
  
  Serial << "Ready! Use 'telnet " << WiFi.localIP() << " 23' to connect" << endl;

}

void loop() {
    static long count = 0;
    if (server.hasClient()){
        buffer << "if (server.hasclient()) => YES" << endl;
        if (!serverClient.connected() || !serverClient){
           buffer << "if (!serverClient.connected() || !serverClient) => YES" << endl;
          if(!serverClient) {
            buffer << "if(!serverClient) => YES" << endl;
            serverClient.stop();
          }
            serverClient = server.available();
            buffer << "server.available" << endl;
            serverClient.flush();
        }
        else  {
            WiFiClient serverClient = server.available();
            serverClient.stop();
        }
   }

   buffer << "Hello World" << endl;
   buffer << ex_text << ex_text2 << " and " << ex_text3 << endl;
   buffer << "This is the loop number: " << count << endl;
   console_send();

   count++;
   

   

   // Sleep 2 sec
   delay(2000);
}


void console_send() {
	
    if (buffer.available()) {
        long chars = buffer.available();
        long i = 0;
        while ( i < chars) {
          
            if (serverClient && serverClient.connected()){
    
                serverClient.write(buffer.peek());
                delay(0);
                
             }
         Serial.write(buffer.read());  
         delay(0); 
         i++;
        }
           
    } 
}
		
	