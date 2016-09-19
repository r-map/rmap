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

