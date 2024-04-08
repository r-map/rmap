#include "common.h"

#ifndef STIMAWIFI_H_
#define STIMAWIFI_H_

const char* update_url = "/firmware/update/" FIRMWARE_TYPE "/";
const uint16_t update_port = 80;

WiFiManager wifiManager;
WebServer webserver(STIMAHTTP_PORT);

WiFiClient httpClient;
WiFiClient mqttClient;
//EspHtmlTemplateProcessor templateProcessor(&server);
MutexStandard loggingmutex;
MutexStandard i2cmutex;
MutexStandard geomutex;

//flag for saving data
bool shouldSaveConfig = false;
bool pmspresent =  false;

U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0);
bool oledpresent=false;

// i2c button for wemos OLED version 2.1.0
I2C_BUTTON button; //I2C address 0x31
// I2C_BUTTON button(DEFAULT_I2C_BUTTON_ADDRESS); //I2C address 0x31

summarydata_t summarydata;

georef_t georef={"","",0,&geomutex};


/*
If the variable can be updated atomically (for example it is not a
32-bit variable on a 16-bit architecture, which would take two writes to
update all 32-bits), and there is only one task that ever writes to the
variable (although many can read from it), then a global variable should
not cause a problem.
This is the case for stimawifiStatus
*/
stimawifiStatus_t stimawifiStatus;

udp_data_t udp_data={1,&frtosLog,&stimawifiStatus.udp,&georef};
udpThread threadUdp(udp_data);

gps_data_t gps_data={1,&frtosLog,&stimawifiStatus.gps,&georef};
gpsThread threadGps(gps_data);

Queue dbQueue((12*2),sizeof(mqttMessage_t));       // ~ 1 minutes queue
Queue mqttQueue((12*3),sizeof(mqttMessage_t));   // ~ 1.5 minutes queue
BinarySemaphore recoverySemaphore(false);
db_data_t db_data={1,&frtosLog,&dbQueue,&mqttQueue,&recoverySemaphore,&stimawifiStatus.db};
dbThread threadDb(db_data);

station_t station;
measure_data_t measure_data={1,&frtosLog,&mqttQueue,&stimawifiStatus.measure,&station,&summarydata,&i2cmutex,&georef};
measureThread threadMeasure(&measure_data);

publish_data_t publish_data={1,&frtosLog,&mqttQueue,&dbQueue,&stimawifiStatus.publish,&station,&mqttClient};
publishThread threadPublish(publish_data);

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, LED_PIN, NEO_GRB + NEO_KHZ800);

char status[15]="";     // status message for web and display


String Json();
String Data();
String FullPage();
void writeconfig();

// web server response function
void handle_FullPage();
void handle_Data();
void handle_Json();
void handle_NotFound();
//callback notifying us of the need to save config
void saveConfigCallback ();
String  rmap_get_remote_config();
void firmware_upgrade();
String readconfig_rmap();
void writeconfig_rmap(const String payload);
int  rmap_config(const String payload);
void readconfig();
void writeconfig();
void web_values(const char* values);
void measureAndPublish();
void reboot();
void logPrefix(Print* _logOutput);
void logSuffix(Print* _logOutput);
void setup();
void loop();

#endif
