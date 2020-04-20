//sensor definition

#define TEMPERATURE_DEFAULTADDRESS 73
#define HUMIDITY_DEFAULTADDRESS 39

#define SAMPLERATE 3000
#define MINUTEFORREPORT 14

// max missing data for minute
#define MAXMISSING 1

// activate debug on serial port
#define DEBUGONSERIAL

#ifdef DEBUGONSERIAL
#define IF_SDEBUG(x) ({x;})
#else
#define IF_SDEBUG(x)
#endif

// set the I2C clock frequency 
#define I2C_CLOCK 30418

#define LEDPIN 13
#define FORCEDEFAULTPIN 8

// define the version of the configuration saved on eeprom
// if you chenge this the board start with default configuration at boot
#define CONFVER "confth00"

// 0: sensor disabled
// 1: sensor enabled
#define USE_SENSORS_ADT   1
#define USE_SENSORS_HIH   1
#define USE_SENSORS_HYT   0

// edit only if new sensor were added, and only after new USE_SENSORS_XXX define for sensor XXX
#define SENSORS_COUNT     (USE_SENSORS_ADT + USE_SENSORS_HIH + USE_SENSORS_HYT)
