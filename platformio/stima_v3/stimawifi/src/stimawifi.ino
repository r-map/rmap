/*
Copyright (C) 2023  Paolo Patruno <p.patruno@iperbole.bologna.it>
authors:
Paolo Patruno <p.patruno@iperbole.bologna.it>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Freeg Software Foundation; either version 2 of 
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
TODO PORTING TO ESP32 WeMos-D1-Mini-ESP32
https://www.dropbox.com/s/4phxfx75hje5eu4/Board-de-desarrollo-WeMos-D1-Mini-ESP32-WiFiBluetooth-BLE-Pines.jpg
https://cdn.shopify.com/s/files/1/1509/1638/files/D1_Mini_ESP32_-_pinout.pdf

* implementing OTA firmware updater for ESP32 (https): now I use a old simple porting of ESP8266httpUpdate
* change second serial port from software to hardware (but it seems implemented in not wemos connector)
* check if LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED) default to do not autoformat littlefs
* wait for new LittleFS release for esp8266 API compatibility  https://github.com/espressif/arduino-esp32/pull/5396
*/

#include "stimawifi.h"
//#include "arduino_thread.h"
//#include "udp_thread.h"
//#include "measure_thread.h"
//#include "publish_thread.h"

sensor_t  sensors[SENSORS_LEN];
SensorDriver* sd[SENSORS_LEN];

const char* update_url = "/firmware/update/" FIRMWARE_TYPE "/";
const uint16_t update_port = 80;

WebServer webserver(STIMAHTTP_PORT);

WiFiClient espClient;
PubSubClient mqttclient(espClient);
//WebSocketsServer webSocket(WS_PORT);
//EspHtmlTemplateProcessor templateProcessor(&server);
WiFiUDP UDP;
OZGPS gps;
MGPS mgps;
MutexStandard loggingmutex;

//flag for saving data
bool shouldSaveConfig = false;
bool pmspresent =  false;

//define your default values here, if there are different values in config.json, they are overwritten.
char rmap_longitude[11] = "";
char rmap_latitude[11] = "";
char rmap_network[31] = "";
char rmap_server[41] = "rmap.cc";
char ntp_server[41] = "europe.pool.ntp.org";
char rmap_mqtt_server[41] = "rmap.cc";
int  rmap_sampletime = DEFAULT_SAMPLETIME;
char rmap_user[10] = "";
char rmap_password[31] = "";
char rmap_slug[31] = "stimawifi";
char rmap_mqttrootpath[10] = "sample";
char rmap_mqttmaintpath[10] = "maint";

U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0);
bool oledpresent=false;
unsigned short int displaypos;

// i2c button for wemos OLED version 2.1.0
I2C_BUTTON button; //I2C address 0x31
// I2C_BUTTON button(DEFAULT_I2C_BUTTON_ADDRESS); //I2C address 0x31
// I2C_BUTTON button(your_address); //using customize I2C address

float temperature=NAN;
int humidity=-999,pm2=-999,pm10=-999,co2=-999;

udp_data_t udp_data={1,frtosLog};
udpThread threadUdp(udp_data);

Queue mqttQueue(10,sizeof(mqttMessage_t));

measure_data_t measure_data={1,frtosLog,mqttQueue};
measureThread threadMeasure(measure_data);

publish_data_t publish_data={1,frtosLog,mqttQueue};
publishThread threadPublish(publish_data);
