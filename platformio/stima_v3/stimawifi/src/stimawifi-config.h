#ifndef STIMAWIFI_CONFIG_H_
#define STIMAWIFI_CONFIG_H_

// increment on change
#define SOFTWARE_VERSION "2023-10-31T00:00"    // date and time
#define MAJOR_VERSION    "20231031"            // date  YYYYMMDD
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
#define WS_PORT 81

// set the frequency
// 30418,25 Hz  : minimum freq with prescaler set to 1 and CPU clock to 16MHz 
#define I2C_CLOCK 10000

//disable debug at compile time but call function anyway
//#define DISABLE_LOGGING disable

#define FIRMWARE_TYPE "WEMOS_D1_MINI32"
#define PMS_RESET 0

#define SCL_PIN SCL
#define SDA_PIN SDA

#define RESET_PIN 4    // pin to connect to ground for reset wifi configuration
// Set LED_BUILTIN if it is not defined by Arduino framework
#ifndef LED_BUILTIN
    #define LED_BUILTIN 2
#endif
#define LED_PIN LED_BUILTIN
// for sensor_t
#define SENSORS_LEN 5
#define SENSORDRIVER_DRIVER_LEN 5
#define SENSORDRIVER_TYPE_LEN 5
#define SENSORDRIVER_META_LEN 30
#define MAX_VALUES_FOR_SENSOR 9

#define CH 9            // character height px

#endif
