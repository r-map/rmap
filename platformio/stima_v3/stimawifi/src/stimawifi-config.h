#ifndef STIMAWIFI_CONFIG_H_
#define STIMAWIFI_CONFIG_H_

// increment on change
#define SOFTWARE_VERSION "2024-01-31T00:00"    // date and time
#define MAJOR_VERSION    "20240131"            // date  YYYYMMDD
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

#define STIMAHTTP_PORT 80

// set the I2c clock frequency
#define I2C_CLOCK 50000

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
#define SENSORS_LEN 5
#define SENSORDRIVER_DRIVER_LEN 5
#define SENSORDRIVER_TYPE_LEN 5
#define SENSORDRIVER_META_LEN 30
#define MAX_VALUES_FOR_SENSOR 9

#define CH 8            // character height px for display

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
