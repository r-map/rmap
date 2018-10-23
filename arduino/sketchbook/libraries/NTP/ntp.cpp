/**@file ntp.cpp */

/*********************************************************************
Copyright (C) 2017  Marco Baldinetti <m.baldinetti@digiteco.it>
authors:
Marco Baldinetti <m.baldinetti@digiteco.it>

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
**********************************************************************/

#include "ntp.h"

void Ntp::makePacket(uint8_t *ntp_packet) {
   memset(ntp_packet, 0, NTP_PACKET_LENGTH);
   ntp_packet[0] = 0b11100011;
   ntp_packet[1] = 0;
   ntp_packet[2] = 6;
   ntp_packet[3] = 0xEC;
   ntp_packet[12]  = 49;
   ntp_packet[13]  = 0x4E;
   ntp_packet[14]  = 49;
   ntp_packet[15]  = 52;
}

uint32_t Ntp::extractTime(uint8_t *ntp_packet) {
   uint32_t seconds_since_1900;
   seconds_since_1900 =  (uint32_t) ntp_packet[NTP_RECEIVE_TIMESTAMP_OFFSET] << 24;
   seconds_since_1900 |= (uint32_t) ntp_packet[NTP_RECEIVE_TIMESTAMP_OFFSET+1] << 16;
   seconds_since_1900 |= (uint32_t) ntp_packet[NTP_RECEIVE_TIMESTAMP_OFFSET+2] << 8;
   seconds_since_1900 |= (uint32_t) ntp_packet[NTP_RECEIVE_TIMESTAMP_OFFSET+3];
   seconds_since_1900 = seconds_since_1900 - NTP_70_YEARS_SECONDS + NTP_TIMEZONE * NTP_1_HOUR_SECONDS;

   if (seconds_since_1900 < NTP_VALID_START_TIME_S) {
      seconds_since_1900 = 0;
   }

   return seconds_since_1900;
}

bool Ntp::sendRequest(EthernetUDP *client, const char *server) {
   uint8_t ntp_buffer[NTP_PACKET_LENGTH];
   makePacket(ntp_buffer);

   client->flush();

   if (!client->beginPacket(server, NTP_SERVER_PORT)) {
      return false;
   }

   if (!client->write(ntp_buffer, NTP_PACKET_LENGTH)) {
      return false;
   }

   if (!client->endPacket()) {
      return false;
   }

   return true;
}

uint32_t Ntp::getResponse (EthernetUDP *client) {
   uint8_t ntp_buffer[NTP_PACKET_LENGTH];
   if (client->parsePacket() < NTP_PACKET_LENGTH) {
      return 0;
   }

   if (client->read(ntp_buffer, NTP_PACKET_LENGTH) <= 0) {
      return 0;
   }

   client->flush();

   return extractTime(ntp_buffer);
}

bool Ntp::sendRequest(sim800Client *client) {
   uint8_t ntp_buffer[NTP_PACKET_LENGTH];
   makePacket(ntp_buffer);
   return client->write(ntp_buffer, NTP_PACKET_LENGTH);
}

uint32_t Ntp::getResponse (sim800Client *client) {
   uint8_t ntp_buffer[NTP_PACKET_LENGTH];

   if (client->readBytes(ntp_buffer, NTP_PACKET_LENGTH) <= 0) {
      return 0;
   }

   return extractTime(ntp_buffer);
}
