#ifndef STIMAWIFI_CONFIG_H_
#define STIMAWIFI_CONFIG_H_

// increment on change
#define SOFTWARE_VERSION "2025-10-21T00:00"    // date and time iso format
#define MAJOR_VERSION    "20251021"            // date  YYYYMMDD
#define MINOR_VERSION    "0"                   // time  HHMM without leading 0

// SSID and password of WiFi for setup
#define WIFI_SSED "STIMA-config"
#define WIFI_PASSWORD  "bellastima"

// defaul sample time to get measure from sensors
#define DEFAULT_SAMPLETIME 30

// udp port to use for communicate with androd gps_forwarder app
#define UDP_PORT 8888

// Serial speed of the GPS

// default for UBLOX NEO-6M with two configuration pin not connected (NMEA messages too)
//#define GPS_SERIAL_SPEED 9600

// default for LC76G(AB) Baudrate 115200bps
#define GPS_SERIAL_SPEED 115200

// display I2C address
#define OLEDI2CADDRESS_64X48 0X3C
#define OLEDI2CADDRESS_128X64 0X3D

// logging level at compile time
// Available levels are:
// LOG_LEVEL_SILENT, LOG_LEVEL_FATAL, LOG_LEVEL_ERROR, LOG_LEVEL_WARNING, LOG_LEVEL_NOTICE, LOG_LEVEL_VERBOSE
#define LOG_LEVEL   LOG_LEVEL_NOTICE

// Length of datetime string %04u-%02u-%02uT%02u:%02u:%02u
#define DATE_TIME_STRING_LENGTH                       (25)

// minimum heap size for warning
#define HEAP_MIN_WARNING 3000

// minimum thread stack size for warning
#define STACK_MIN_WARNING 100

// port for http server
#define STIMAHTTP_PORT 80

// set to 1 if we use PMS sensor
#define PMS_RESET 0

// define pins for I2C
#define SCL_PIN SCL
#define SDA_PIN SDA

// define pin for reset
#define RESET_PIN D9

// define reset and LED pins
#if defined(ARDUINO_LOLIN_C3_MINI)
//C3 mini
#define FIRMWARE_TYPE "LOLIN_C3_MINI"
// https://www.wemos.cc/en/latest/_static/files/sch_c3_mini_v2.1.0.pdf
// pin to connect to ground for reset wifi configuration
#define LED_PIN D3
#endif

#if defined(ARDUINO_LOLIN_S3_MINI)
//S3 mini
#define FIRMWARE_TYPE "LOLIN_S3_MINI"
// https://www.wemos.cc/en/latest/_static/files/sch_s3_mini_v1.0.0.pdf
#define LED_PIN 47
#endif

// size for sensor_t
#define SENSORDRIVER_DRIVER_LEN 5
#define SENSORDRIVER_TYPE_LEN 5
#define SENSORDRIVER_META_LEN 30

// define parameter for queues len and communication
//#define DATA_BURST (SENSORS_MAX*VALUES_TO_READ_FROM_SENSOR_COUNT)
#define DATA_BURST (15)
#define DATA_BURST_RECOVERY (DATA_BURST)

#define DB_QUEUE_LEN (DATA_BURST)
#define MQTT_QUEUE_LEN (DATA_BURST*3)

#define MQTT_QUEUE_SPACELEFT_MEASURE (DATA_BURST)
#define MQTT_QUEUE_SPACELEFT_PUBLISH (DATA_BURST/2)
#define MQTT_QUEUE_SPACELEFT_RECOVERY (DATA_BURST*2)

// SD card SPI PIN assignment
// Micro SD Card Shield
#define C3SCK D5   
#define C3MISO D6  
#define C3MOSI D7

// https://www.wemos.cc/en/latest/d1_mini_shield/micro_sd.html
// configuration as default
// #define C3SS D4

// WIFI D1 mini - Data logger shield for D1 mini with RTC and MicroSD
// https://en.m.nu/esp8266-shields/wifi-d1-mini-data-logger-shield-for-d1-mini-with-rtc-and-microsd
// or
// https://www.wemos.cc/en/latest/d1_mini_shield/micro_sd.html
// configurated as
// cut 2/D4 and connect 15/18
#define C3SS D8

// SPI clock
#define SPICLOCK 10000000

// SD card max number of file opened

// permanent opened files:
// 1 sqlite
// 1 loggin on sdcard (if defined by macro)
// 1 RPC recovery (archive) not permanent but for a long time 

// temporary opened files:
// 1 WEB response archive file
// 1 WEB response info file
// 2 SQLITE temporary files (??) https://www.sqlite.org/tempfiles.html

#define SDMAXFILE 6

// SD card file name for archive
#define SDCARD_INFO_FILE_NAME    ("/info.dat")
#define SDCARD_ARCHIVE_FILE_NAME ("/archive.dat")

// littlefs max number of file opened on EEPROM
#define LFMAXFILE 4

// time in seconds saved on sqlite on SD card for tmp archive and recovery
#define SDRECOVERYTIME (3600*24*1)

// sqlite setup
/*
  https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/memory-types.html
  There is 520 KB of available SRAM (320 KB of DRAM and 200 KB of
  IRAM) on the ESP32. However, due to a technical limitation, the
  maximum statically allocated DRAM usage is 160 KB. The remaining 160
  KB (for a total of 320 KB of DRAM) can only be allocated at runtime
  as heap.
*/
#define SQLITE_MEMORY 110000

/*!
\def MQTT_TIMEOUT_MS
\brief Timeout in milliseconds for mqtt stack.
*/
#define MQTT_TIMEOUT_MS                (6000)

/*!
\def IP_STACK_TIMEOUT_MS
\brief IPStack timeout.
*/
#define IP_STACK_TIMEOUT_MS            (MQTT_TIMEOUT_MS)

/*!
\def STATUS_SEND_S
\brief send MQTT board status every STATUS_SEND_S.
*/
#define STATUS_SEND_S (3600)

/*!
\def MQTT_PACKET_SIZE
\brief Length in bytes for max mqtt packet size.
*/
#define MQTT_PACKET_SIZE               (220)

// MQTT broker port
# define MQTT_SERVER_PORT (1883)

/*!
\def CONSTANTDATA_BTABLE_LENGTH
\brief Maximum lenght of btable code plus terminator that describe one constant data.
*/
#define CONSTANTDATA_BTABLE_LENGTH                    (7)

/*!
\def CONSTANTDATA_VALUE_LENGTH
\brief Maximum lenght of value plus terminator for one constant data.
*/
#define CONSTANTDATA_VALUE_LENGTH                    (33)

/*!
\def OUTPUTPINS
\brief Define the pins numbers for use with outputpins RPC
*/
#define OUTPUTPINS TX,RX,D0,D4

// task definitions
// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/freertos.html#background-tasks
// https://github.com/espressif/esp-idf/blob/master/components/esp_system/include/esp_task.h
// https://github.com/espressif/esp-idf/blob/master/components/freertos/config/include/freertos/FreeRTOSConfig.h

#define TASK_UDP_PRIORITY           4
#define TASK_UDP_STACK_SIZE         2000

#define TASK_DB_PRIORITY            3
#define TASK_DB_STACK_SIZE          5500

#define TASK_MEASURE_PRIORITY       2
#define TASK_MEASURE_STACK_SIZE     4000

#define TASK_GPS_PRIORITY           2
#define TASK_GPS_STACK_SIZE         2500

#define TASK_PUBLISH_PRIORITY       1
#define TASK_PUBLISH_STACK_SIZE     3500

#define TASK_LOOP_PRIORITY          1
//#define ARDUINO_LOOP_STACK_SIZE   5000  // defined in platformio.ini

#endif
