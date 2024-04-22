#ifndef STIMAWIFI_CONFIG_H_
#define STIMAWIFI_CONFIG_H_

// increment on change
#define SOFTWARE_VERSION "2024-04-01T00:00"    // date and time
#define MAJOR_VERSION    "20240401"            // date  YYYYMMDD
#define MINOR_VERSION    "0"                   // time  HHMM without leading 0

#define WIFI_SSED "STIMA-config"
#define WIFI_PASSWORD  "bellastima"
#define DEFAULT_SAMPLETIME 30
#define UDP_PORT 8888

#define OLEDI2CADDRESS 0X3C

// logging level at compile time
// Available levels are:
// LOG_LEVEL_SILENT, LOG_LEVEL_FATAL, LOG_LEVEL_ERROR, LOG_LEVEL_WARNING, LOG_LEVEL_NOTICE, LOG_LEVEL_VERBOSE
#define LOG_LEVEL   LOG_LEVEL_NOTICE
// Length of datetime string %04u-%02u-%02uT%02u:%02u:%02u
#define DATE_TIME_STRING_LENGTH                       (25)

// minimum heap size for warning
#define HEAP_MIN_WARNING 20000

// minimum thread stack size for warning
#define STACK_MIN_WARNING 100

#define STIMAHTTP_PORT 80

#define FIRMWARE_TYPE "LOLIN_C3_MINI"
#define PMS_RESET 0

#define SCL_PIN SCL
#define SDA_PIN SDA

#if defined(ARDUINO_LOLIN_C3_MINI)
//C3 mini
// https://www.wemos.cc/en/latest/_static/files/sch_c3_mini_v2.1.0.pdf
// pin to connect to ground for reset wifi configuration
#define RESET_PIN 4
#define LED_PIN 7
#endif

#if defined(ARDUINO_LOLIN_S3_MINI)
//S3 mini
// https://www.wemos.cc/en/latest/_static/files/sch_s3_mini_v1.0.0.pdf
#define RESET_PIN 11
#define LED_PIN 47
#endif

#if defined(ARDUINO_D1_MINI32)
// D1 mini ESP32
// https://cdn.shopify.com/s/files/1/1509/1638/files/D1_Mini_ESP32_-_pinout.pdf?v=1604068668
#define RESET_PIN 23
#define LED_PIN 34       // not connected to neopixel
#endif

// for sensor_t
#define SENSORDRIVER_DRIVER_LEN 5
#define SENSORDRIVER_TYPE_LEN 5
#define SENSORDRIVER_META_LEN 30

#define CH 8            // character height px for display

//#define DATA_BURST (SENSORS_MAX*VALUES_TO_READ_FROM_SENSOR_COUNT)
#define DATA_BURST (15)
#define DB_QUEUE_LEN (DATA_BURST)
#define MQTT_QUEUE_LEN (DATA_BURST*2)
#define MQTT_QUEUE_SPACELEFT_RECOVERY (MQTT_QUEUE_LEN/3*2)
#define MQTT_QUEUE_SPACELEFT_PUBLISH (MQTT_QUEUE_LEN/2)
#define MQTT_QUEUE_BURST_RECOVERY (DATA_BURST/2)

// SD card SPI PIN assignment
#define C3SCK 1   
#define C3MISO 0  
#define C3MOSI 4  
#define C3SS 6    

// SD card max number of file
#define MAXFILE 4

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
\def MQTT_PACKET_SIZE
\brief Length in bytes for max mqtt packet size.
*/
#define MQTT_PACKET_SIZE               (220)

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


#endif
