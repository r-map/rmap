#include <sensors_config.h>
#include <mqtt_config.h>
#include <constantdata_config.h>
#include <gsm_config.h>
#include <ntp_config.h>
#include <ntp_config.h>
#include <typedef.h>


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

