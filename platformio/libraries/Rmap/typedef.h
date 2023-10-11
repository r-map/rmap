/**@file typedef.h */

/*********************************************************************
Copyright (C) 2017  Marco Baldinetti <m.baldinetti@digiteco.it>
authors:
Paolo Patruno <p.patruno@iperbole.bologna.it>
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

#ifndef _TYPEDEF_H
#define _TYPEDEF_H

#include <mqtt_config.h>
#include <constantdata_config.h>
#include <gsm_config.h>
#include <ntp_config.h>

/*!
\def DRIVER_LENGTH
\brief Sensor driver's buffer length.
*/
#define DRIVER_LENGTH       (5)

/*!
\def TYPE_LENGTH
\brief Sensor type's buffer length.
*/
#define TYPE_LENGTH         (5)



/*!
\struct sensor_t
\brief Sensor struct for storing sensor configuration parameter.
*/
typedef struct {
  char driver[DRIVER_LENGTH];            //!< sensor's string driver
  char type[TYPE_LENGTH];                //!< sensor's string type
  uint8_t address;                              //!< sensor's address
  uint8_t node;                                 //!< sensor's node
  char mqtt_topic[MQTT_SENSOR_TOPIC_LENGTH];    //!< sensor's mqtt topic path
} sensor_t;

/*!
\struct rain_t
\brief Rain tips struct for storing rain data.
*/
typedef struct {
  uint16_t tips_count;  //!< rain gauge tips counter
  uint16_t rain;        //!< rain (Hg/m^2)
} rain_t;

/*!
\struct value_t
\brief Value struct for storing sample, observation and minium, average and maximum measurement.
*/
typedef struct {
  uint16_t sample; //!< last sample
  uint16_t med60;  //!< last observation
  uint16_t med;    //!< average values of observations
  uint16_t max;    //!< maximum values of observations
  uint16_t min;    //!< minium values of observations
  uint16_t sigma;  //!< standard deviation of observations
} value_t;


/*********************************************************************
* TYPEDEF
*********************************************************************/

/*!
\struct sensordata_t
\brief constant station data (station name, station height ...) parameters.
*/
typedef struct {
   char btable[CONSTANTDATA_BTABLE_LENGTH];                 //!< table B code for constant station data
   char value[CONSTANTDATA_VALUE_LENGTH];                   //!< value of constant station data
} constantdata_t;

/*!
\struct configuration_t
\brief EEPROM saved configuration.
*/
typedef struct {
   uint8_t module_main_version;                             //!< module main version
   uint8_t module_configuration_version;                    //!< module configuration version
   uint8_t module_type;                                     //!< module type

   #if (USE_MQTT)
   uint16_t mqtt_port;                                      //!< mqtt server port
   char mqtt_server[MQTT_SERVER_LENGTH];                    //!< mqtt server
   char mqtt_root_topic[MQTT_ROOT_TOPIC_LENGTH];            //!< mqtt root path
   char mqtt_maint_topic[MQTT_MAINT_TOPIC_LENGTH];          //!< mqtt maint path
   char mqtt_rpc_topic[MQTT_RPC_TOPIC_LENGTH];              //!< mqtt subscribe topic
   char mqtt_username[MQTT_USERNAME_LENGTH];                //!< username to compose mqtt username (username/stationslug/boardslug)
   char mqtt_password[MQTT_PASSWORD_LENGTH];                //!< mqtt password
   char stationslug[STATIONSLUG_LENGTH];                    //!< station slug to compose mqtt username (username/stationslug/boardslug)
   char boardslug[BOARDSLUG_LENGTH];                        //!< board slug to compose mqtt username (username/stationslug/boardslug)
   #endif

   #if (USE_NTP)
   char ntp_server[NTP_SERVER_LENGTH];                      //!< ntp server
   #endif

   sensor_t sensors[SENSORS_MAX];                           //!< SensorDriver buffer for storing sensors parameter
   uint8_t sensors_count;                                   //!< configured sensors number
   uint16_t report_seconds;                                 //!< seconds for report values

   constantdata_t constantdata[USE_CONSTANTDATA_COUNT];     //!< Constantdata buffer for storing constant station data parameter
   uint8_t constantdata_count;                              //!< configured constantdata number
  
   #if (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_ETH || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_ETH)
   bool is_dhcp_enable;                                     //!< dhcp status
   uint8_t ethernet_mac[ETHERNET_MAC_LENGTH];               //!< ethernet mac
   uint8_t ip[ETHERNET_IP_LENGTH];                          //!< ip address
   uint8_t netmask[ETHERNET_IP_LENGTH];                     //!< netmask
   uint8_t gateway[ETHERNET_IP_LENGTH];                     //!< gateway
   uint8_t primary_dns[ETHERNET_IP_LENGTH];                 //!< primary dns

   #elif (MODULE_TYPE == STIMA_MODULE_TYPE_SAMPLE_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_REPORT_GSM || MODULE_TYPE == STIMA_MODULE_TYPE_PASSIVE_GSM)
   char gsm_apn[GSM_APN_LENGTH];                            //!< gsm apn
   char gsm_username[GSM_USERNAME_LENGTH];                  //!< gsm username
   char gsm_password[GSM_PASSWORD_LENGTH];                  //!< gsm password

   #endif
} configuration_t;


#endif
