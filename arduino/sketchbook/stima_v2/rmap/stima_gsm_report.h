/*
Copyright (C) 2016  Paolo Paruno <p.patruno@iperbole.bologna.it>
authors:
Paolo Paruno <p.patruno@iperbole.bologna.it>

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

///////////////////////////////////////////////////////////////////////
// Compile time configuration

#define FIRMWARE FIRMGSM
/////////////////////////


// activate if you have an rtc present (too big program fo arduino uno)
//#define RTCPRESENT
// activate if you have an GPS + dedicated arduino for I2C query
//#define I2CGPSPRESENT
//#define I2C_GPS_ADDRESS 0x20
// activate if you have ENC28J60 for ethernet
//#define USE_ENC28J60
// activate if you have the Hobby Components I2C LCD module (HCARDU0023)
// LCD Version 1 - Marked YwRobot Arduino LCM1602 IIC V1
#define LCD
// activate if you have relays connected to some pins
#define ATTUATORE

#define REPORTMODE

///////////////////////////////////////////////////////////////////////

#include "common.h"

// override default values defined in common.h
// for sensoron
//#define SENSORS_LEN 1

//#define MQTTROOTPATH_LEN 30
//#define SERVER_LEN 30

#define MAIN_BUFFER_SIZE 300

// force use of gsm mqtt
#undef ETHERNETMQTT
#undef GSMGPRSHTTP
#define GSMGPRSMQTT


#ifdef REPORTMODE
  // timing for REPORT MODE
  #define MQTPUBLISH_TIME 45
  #define MGRSDCARD_TIME 60
  #define MQTTCONNECT_TIME 75
  #define MQTTCONNECT_ONCE_TIME 75
#endif
