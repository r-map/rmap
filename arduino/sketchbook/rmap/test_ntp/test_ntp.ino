/*
Copyright (C) 2015  Paolo Paruno <p.patruno@iperbole.bologna.it>
authors:
Paolo Patruno <p.patruno@iperbole.bologna.it>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of 
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define UIPETHERNET_DEBUG_UDP 1

#include <UIPEthernet.h>
#include <UIPUdp.h>
#include <Time.h>
#include <SPI.h>         
#include <Dns.h>

byte mac[6]={0x74,0xd0,0x2b,0x24,0x24,0xff};                           // Ethernet mac address
// A UDP and TCP instance
EthernetUDP Udp;


char ntpserver[]="pool.ntp.org";
//char ntpserver[]="pat1";
//byte ntpserver[]={192,168,1,1};
time_t t;


// utility function to debug
void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

/*
 Udp NTP Client
 Get the time from a Network Time Protocol (NTP) time server

 created 4 Sep 2010 
 by Michael Margolis
 modified 9 Apr 2012
 by Tom Igoe
*/

#define NTP_PACKET_SIZE 48               // NTP time stamp is in the first 48 bytes of the message

// send an NTP request to the time server
//unsigned long sendNTPpacket(IPAddress& address)
void sendNTPpacket()
{
  byte packetBuffer[ NTP_PACKET_SIZE];        //buffer to hold incoming and outgoing packets 

  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE); 
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49; 
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp: 		   
  Udp.beginPacket(ntpserver, 123); //NTP requests are to port 123
  Udp.write(packetBuffer,NTP_PACKET_SIZE);
  Udp.endPacket(); 
  Serial.println(F("#ntp packet sended"));
}

time_t receiveNTPpacket(){
  
  byte packetBuffer[ NTP_PACKET_SIZE];        //buffer to hold incoming and outgoing packets 

  int count  = 0;
  while (  count++  < 10 ){
    if ( Udp.parsePacket() > 0 ) {
      // We've received a packet, read the data from it
      if (Udp.read(packetBuffer,NTP_PACKET_SIZE)< NTP_PACKET_SIZE ){   // read the packet into the buffer
	Serial.println(F("#error getting short ntp packet response"));
	return 0UL;
      }

      //the timestamp starts at byte 40 of the received packet and is four bytes,
      // or two words, long. First, esxtract the two words:
      
      unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
      unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);  
      // combine the four bytes (two words) into a long integer
      // this is NTP time (seconds since Jan 1 1900):
      unsigned long secsSince1900 = highWord << 16 | lowWord;  
      Serial.print(F("#NTP response: seconds since Jan 1 1900 = "));
      Serial.println(secsSince1900);
      const unsigned long seventy_years = 2208988800UL;
      return secsSince1900 -  seventy_years;
    }
    delay(500);
    Serial.println("delay 500");
  }
      
  Serial.println(F("#error getting ntp packet response"));
  return 0UL;
}

time_t getNtpTime()
{
  sendNTPpacket();            // send an NTP packet to a time server
  return receiveNTPpacket();  // get a reply if it is available
}

// Just a utility function to nicely format an IP address.
const char* ip_to_str(const IPAddress& ipAddr)
{
  static char buf[16];
  sprintf(buf, "%d.%d.%d.%d", ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
  return buf;
}

void digitalClockDisplay(time_t t ){
  // digital clock display of the time

  Serial.print(F("#"));

  if (t == 0UL){
    Serial.println(F("NULL"));
  }
  else{	  
    printDigits(hour(t));
    Serial.print(":");
    printDigits(minute(t));
    Serial.print(":");
    printDigits(second(t));
    Serial.print(" ");
    printDigits(day(t));
    Serial.print(" ");
    printDigits(month(t));
    Serial.print(" ");
    Serial.print(year(t)); 
    Serial.println(); 
  }
}

void setup() 
{

  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  Serial.println("started");

  SPI.begin();
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(SS, OUTPUT);


  while (Ethernet.begin(mac,8) == 0) {
    Serial.println(F("#Failed to configure Ethernet using DHCP"));
  }

  //IPAddress myIP(192,168,1,6);
  //Ethernet.begin(mac,myIP,8);

  Serial.print(F("#My IP address is "));
  Serial.println(ip_to_str(Ethernet.localIP()));
  
  Serial.print(F("#Netmask is "));
  Serial.println(ip_to_str(Ethernet.subnetMask()));
  
  Serial.print(F("#Gateway IP address is "));
  Serial.println(ip_to_str(Ethernet.gatewayIP()));
  
  Serial.print(F("#DNS IP address is "));
  Serial.println(ip_to_str(Ethernet.dnsServerIP()));


  IPAddress otherIP(0,0,0,0);
  DNSClient dns;
  dns.begin(Ethernet.dnsServerIP());
  Serial.println(dns.getHostByName("pool.ntp.org", otherIP));
  Serial.println(ip_to_str(otherIP));

  Udp.begin(123);

  setSyncProvider(getNtpTime);
}


void loop()
{  

  if (timeStatus() == timeNeedsSync) {
    Serial.print(F("#The time had been set but a sync attempt did not succeed"));
    Serial.println(timeStatus());
    t = 0; // Store the current time in time variable t 
  }

  if(timeStatus()== timeNotSet) {
    Serial.println(F("#The time has never been set"));
    t = 0; // Store the current time in time variable t 
  }else{
    t = now(); // Store the current time in time variable t 
  }

  digitalClockDisplay(t);

  // Allows for the renewal of DHCP leases
  switch ( Ethernet.maintain() ) {
  case 0 : 
    Serial.println(F("#ethernet maintain: nothing happened"));
    break;
  case 1 : 
    Serial.println(F("#ethernet maintain: renew failed"));
    break;
  case 2 : 
    Serial.println(F("#ethernet maintain: renew success"));
    break;
  case 3 : 
    Serial.println(F("#ethernet maintain: rebind fail"));
    break;
  case 4 : 
    Serial.println(F("#ethernet maintain: rebind success"));
    break;
    //  default : 
  }

  delay(1000);

}
