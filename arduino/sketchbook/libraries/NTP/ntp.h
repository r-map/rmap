/**@file ntp.h */

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

#ifndef _NTP_H
#define _NTP_H

/*!
\def NTP_SERVER_PORT
\brief NTP server port.
*/
#define NTP_SERVER_PORT                (123)

/*!
\def NTP_PACKET_LENGTH
\brief NTP packet length.
*/
#define NTP_PACKET_LENGTH              (48)

/*!
\def NTP_RECEIVE_TIMESTAMP_OFFSET
\brief NTP received timestamp offset.
*/
#define NTP_RECEIVE_TIMESTAMP_OFFSET   (40)

/*!
\def NTP_1_HOUR_SECONDS
\brief seconds in one hour.
*/
#define NTP_1_HOUR_SECONDS             (3600UL)

/*!
\def NTP_70_YEARS_SECONDS
\brief seconds in 70 years.
*/
#define NTP_70_YEARS_SECONDS           (2208988800UL)

/*!
\def NTP_VALID_START_TIME_S
\brief seconds for 00:00:00 01/01/2017 since 00:00:00 01/01/1970.
*/
#define NTP_VALID_START_TIME_S         (1483228800UL)

/*!
\def NTP_TIMEZONE
\brief NTP timezone is set to GMT.
*/
#define NTP_TIMEZONE                   (0)

#include <stdint.h>
#include <string.h>
#include <EthernetUdp2.h>
#include <sim800Client.h>

/*!
\class Ntp
\brief Ntp class.
*/
class Ntp {
public:
   /*!
   \fn bool sendRequest(EthernetUDP *client, const char *server)
   \brief Send ntp request over ethernet client.
   \param[in] *client ethernet client pointer.
   \param[in] *server ntp server.
   \return true if request was sent.
   */
   static bool sendRequest(EthernetUDP *client, const char *server);

   /*!
   \fn uint32_t getResponse(EthernetUDP *client)
   \brief Get ntp response.
   \param[in] *client ethernet client pointer.
   \return ntp time in seconds since 01/01/1970 00:0:00.
   */
   static uint32_t getResponse(EthernetUDP *client);

   /*!
   \fn bool sendRequest(sim800Client *client)
   \brief Send ntp request over sim800 client.
   \param[in] *client sim800 client pointer.
   \return true if request was sent.
   */
   static bool sendRequest(sim800Client *client);

   /*!
   \fn uint32_t getResponse(sim800Client *client)
   \brief Get ntp response.
   \param[in] *client sim800 client pointer.
   \return ntp time in seconds since 01/01/1970 00:0:00.
   */
   static uint32_t getResponse(sim800Client *client);

private:
   /*!
   \fn void makePacket(uint8_t *ntp_packet)
   \brief Make ntp packet.
   \param[out] *ntp_packet buffer filled with ntp packet.
   \return void.
   */
   static void makePacket(uint8_t *ntp_packet);

   /*!
   \fn uint32_t extractTime(uint8_t *ntp_packet)
   \brief Extract time from ntp packet response.
   \param[in] *ntp_packet ntp packet response.
   \return ntp time in seconds since 01/01/1970 00:0:00.
   */
   static uint32_t extractTime(uint8_t *ntp_packet);
};

#endif
