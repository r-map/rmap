/*
 * UIPEthernet EchoServer example.
 *
 * UIPEthernet is a TCP/IP stack that can be used with a enc28j60 based
 * Ethernet-shield.
 *
 * UIPEthernet uses the fine uIP stack by Adam Dunkels <adam@sics.se>
 *
 *      -----------------
 *
 * This Hello World example sets up a server at 192.168.1.6 on port 1000.
 * Telnet here to access the service.  The uIP stack will also respond to
 * pings to test if you have successfully established a TCP connection to
 * the Arduino.
 *
 * This example was based upon uIP hello-world by Adam Dunkels <adam@sics.se>
 * Ported to the Arduino IDE by Adam Nielsen <malvineous@shikadi.net>
 * Adaption to Enc28J60 by Norbert Truchsess <norbert.truchsess@t-online.de>
 */

#include <SPI.h>
#include <EthernetENC.h>

EthernetServer server = EthernetServer(1000);

void setup()
{
  Serial.begin(9600);

  Serial.println("Started");

  uint8_t mac[6] = {0x00,0x01,0x02,0x03,0x04,0x05};


  SPI.begin();
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(SS, OUTPUT);

  //IPAddress myIP(192,168,1,6);
  //Ethernet.begin(mac,myIP,8);
  Ethernet.begin(mac,8);

  Serial.println("Ethernet started");

  server.begin();

  Serial.println("Server started");

}

void loop()
{
  size_t size;

  if (EthernetClient client = server.available())
    {
      if (client)
        {
          while((size = client.available()) > 0)
            {
              uint8_t* msg = (uint8_t*)malloc(size);
              size = client.read(msg,size);
              Serial.write(msg,size);
              client.write(msg,size);
              free(msg);
            }
        }
    }
}
