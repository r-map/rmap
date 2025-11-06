#include "common.h"
#include "esp_phy_init.h"

/*
*  Print last reset reason of ESP32
*  =================================
*/
#if CONFIG_IDF_TARGET_ESP32  // ESP32/PICO-D4
#include "esp32/rom/rtc.h"
#elif CONFIG_IDF_TARGET_ESP32S2
#include "esp32s2/rom/rtc.h"
#elif CONFIG_IDF_TARGET_ESP32C2
#include "esp32c2/rom/rtc.h"
#elif CONFIG_IDF_TARGET_ESP32C3
#include "esp32c3/rom/rtc.h"
#elif CONFIG_IDF_TARGET_ESP32S3
#include "esp32s3/rom/rtc.h"
#elif CONFIG_IDF_TARGET_ESP32C6
#include "esp32c6/rom/rtc.h"
#elif CONFIG_IDF_TARGET_ESP32H2
#include "esp32h2/rom/rtc.h"
#elif CONFIG_IDF_TARGET_ESP32P4
#include "esp32p4/rom/rtc.h"
#else
#error Target CONFIG_IDF_TARGET is not supported
#endif


#ifndef STIMAWIFI_H_
#define STIMAWIFI_H_

const char* update_url = "/firmware/update/" FIRMWARE_TYPE "/";
const uint16_t update_port = 80;

WiFiManager wifiManager;
WebServer webserver(STIMAHTTP_PORT);

//EspHtmlTemplateProcessor templateProcessor(&server);
MutexStandard loggingmutex;
MutexStandard i2cmutex;
MutexStandard geomutex;

frtosRtc frtosRTC;

#if (ENABLE_SDCARD_LOGGING)   

/*!
\var logFile
\brief File for logging on SD-Card.
*/
File logFile;

/*!
\var loggingStream
\brief stream for logging on Serial and  SD-Card together.
*/
WriteLoggingStream loggingStream(logFile,Serial);
#endif

//flag for saving data
bool shouldSaveConfig = false;
bool pmspresent =  false;
bool oledpresent=false;

// oled display driver pointer
U8G2* u8g2;

// character height px for display
uint8_t CH;

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
station_t station;

udp_data_t udp_data={1,&frtosLog,&stimawifiStatus.udp,&georef,&frtosRTC};
udpThread threadUdp(&udp_data);

gps_data_t gps_data={1,&frtosLog,&stimawifiStatus.gps,&georef,&frtosRTC};
gpsThread threadGps(&gps_data);

Queue dbQueue(DB_QUEUE_LEN,sizeof(mqttMessage_t));       // ~ 1 minutes queue
Queue mqttQueue(MQTT_QUEUE_LEN,sizeof(mqttMessage_t));   // ~ 1.5 minutes queue
BinaryQueue recoveryQueue(sizeof(rpcRecovery_t));
BinarySemaphore recoverySemaphore(false);
#if (ENABLE_SDCARD_LOGGING)
db_data_t db_data={1,&frtosLog,&dbQueue,&mqttQueue,&recoverySemaphore,&recoveryQueue,&stimawifiStatus.db,&station,&logFile};
#else
db_data_t db_data={1,&frtosLog,&dbQueue,&mqttQueue,&recoverySemaphore,&recoveryQueue,&stimawifiStatus.db,&station,NULL};
#endif

dbThread threadDb(&db_data);

measure_data_t measure_data={1,&frtosLog,&mqttQueue,&dbQueue,&stimawifiStatus.measure,&station,&summarydata,&i2cmutex,&georef};
measureThread threadMeasure(&measure_data);

publish_data_t publish_data={1,&frtosLog,&mqttQueue,&dbQueue,&recoveryQueue,&stimawifiStatus,&station};
publishThread threadPublish(&publish_data);

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, LED_PIN, NEO_GRB + NEO_KHZ800);

char status[15]="";     // status message for web and display
int  rssi=0;            // WiFi RSSI for web and display

bool loopinit=true;     // we need initialization of loop task

time_t rtc_set_time();

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
String  get_remote_rmap_config();
void firmware_upgrade();
String read_local_rmap_config();
bool write_local_rmap_config(const String payload);
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
