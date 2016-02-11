/*
Copyright (C) 2014  Paolo Paruno <p.patruno@iperbole.bologna.it>
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

// common configuration for all board

#include "version.h"
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

///////////////////////// those are alternative !
//Arduino Mega2560 switch
//#define FIRMWARE FIRMALL
// Arduino UNO switch
// arduino uno configuration and data on serial port
//#define FIRMWARE FIRMSERIAL
// arduino 644 data on mqtt over ethernet
//#define FIRMWARE FIRMETHERNET
// arduino 644 data on radio rf24
//#define FIRMWARE FIRMRADIORF24
// arduino 644 data with GSM module
//#define FIRMWARE FIRMGSM
/////////////////////////

// output debug messages on serial port
#define DEBUGONSERIAL

///////////////////////////////////////////////////////////////////////

#if FIRMWARE == FIRMALL
#if SERIAL_BUFFER_SIZE == 64
ATTENTION ! remember to change:
#define SERIAL_BUFFER_SIZE 200
in 
HardwareSerial.cpp
if arduino >= 1.5
#define SERIAL_TX_BUFFER_SIZE 192
#define SERIAL_RX_BUFFER_SIZE 192
in
HardwareSerial.h
or set 
compiler.cpp.extra_flags= -DSERIAL_TX_BUFFER_SIZE=192 -DSERIAL_RX_BUFFER_SIZE=192
in platform.local.txt
in sketchbook/hardware/Microduino/avr
https://github.com/arduino/Arduino/wiki/Boards-Manager-FAQ
#endif
#endif


#if FIRMWARE == FIRMSERIAL
#define JSONRPCON
#define SENSORON
#endif

#if FIRMWARE == FIRMETHERNET
#define JSONRPCON
#define ETHERNETON
#define REPEATTASK
#define SENSORON
#endif

#if FIRMWARE == FIRMALL
#define JSONRPCON
#define ETHERNETON
#define RADIORF24
#define REPEATTASK
#define SENSORON
#define NTPON
#endif

#if FIRMWARE == FIRMRADIORF24
// activate if you have rf24 radio board
#define RADIORF24
#define JSONRPCON
#define SENSORON
#endif

#if FIRMWARE == FIRMGSM
// activate if you have GSM GPRS module
#define JSONRPCON
#define REPEATTASK
#define SENSORON
#define GSMAPN "ibox.tim.it"
#define GSMUSER ""
#define GSMPASSWORD ""
//#define GSMSERIAL   Serial1
#define GSMONOFFPIN 0
#define GSMRESETPIN 6
#ifdef REPEATTASK
#define SDCARD
#define SDCHIPSELECT 7
#define GSMGPRSMQTT
#define GSMGPRSRTC
#define GSMGPRSRTCBOOT
#define GSMGPRSHTTP
#endif
// GSMGPRSMQTT and GSMGPRSRTC are mutual exclusive
#ifdef GSMGPRSMQTT
#undef GSMGPRSRTC
#endif

// GSMGPRSRTCBOOT and GSMGPRSRTC are mutual exclusive
#ifdef GSMGPRSRTCBOOT
#undef GSMGPRSRTC
#endif

// GSMGPRSMQTT and GSMGPRSHTTP are mutual exclusive
#ifdef GSMGPRSMQTT
#undef GSMGPRSHTTP
#endif
#endif

//############################################################################

#ifdef JSONRPCON
#define SERIALJSONRPC
#ifdef RADIORF24
#define RF24JSONRPC
#ifdef RF24SLEEP
#include <avr/sleep.h>

/*
Board	          int.0	    int.1	int.2	int.3	int.4	int.5
Uno, Ethernet	      2 	3	 	 	 	 
Mega2560	      	        	   21	   20	   19	   18
*/
#define INTERU    1
#define INTERUPIN 3
//#define INTERU    5
//#define INTERUPIN 18
#endif
#endif
#endif


#ifdef SERIALJSONRPC
#define RPCSERIAL Serial
#define RPCSERIALBAUDRATE 9600
#endif


#ifdef ETHERNETON
#define ETHERNETMQTT
#ifdef JSONRPCON
#define TCPSERVER
#endif
#endif

#ifdef REPEATTASK
//#define SDCARD
//#define SDCHIPSELECT 7
#endif

#ifdef DEBUGONSERIAL
// simple memory stats
//#define FREERAM

// complex memory stats
//#define FREEMEM

#define DBGSERIAL Serial
#define DBGSERIALBAUDRATE 9600

#define IF_SDEBUG(x) ({x;})
#else
#define IF_SDEBUG(x)
#endif

#ifdef LCD
#define IF_LCD(x) ({x;})
#else
#define IF_LCD(x)
#endif


// RF24 pins
#define RF24CEPIN 9  //	The pin attached to Chip Enable on the RF module
#define RF24CSPIN 10 //	The pin attached to Chip Select

// define the version of the configuration saved on eeprom
// if you chenge this the board stop waiting configuration when boot
#define CONFVER "conf00"

// ENC pins
#define ENCCEPIN 8  //	The pin attached to Chip Enable on the ENC module

// define ethernet port for tcp/ip transport
#define ETHERNETPORT 1000

// define the output pins used for (relays)
#define OUTPUTPINS 4,5,A6,A7

// define pin and led that force and display configuration status at boot
#define FORCECONFIGPIN 8          // the same pin of CS for ENC28j60 ENC28J60_CONTROL_CS
#define FORCECONFIGLED 13

// for sensoron
#define SENSORS_LEN 5
#define SENSORDRIVER_DRIVER_LEN 5
#define SENSORDRIVER_TYPE_LEN 5
#define SENSORDRIVER_MQTTPATH_LEN 30

#define MQTTROOTPATH_LEN 100
#define SERVER_LEN 30

#define MAIN_BUFFER_SIZE 192

// set the frequensy 
#define I2C_CLOCK 50000

// MQTT subscribe topic prefix for RPC
#define MQTTRPCPREFIX "rpc/"

// required here to have arduino ide 1.6 happy
// buffer for aJson print output and internal global_buffer
static char mainbuf[MAIN_BUFFER_SIZE];

