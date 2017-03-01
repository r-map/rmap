// sds011 sensor definition

//// take one measure every SAMPLERATE us
#define SAMPLERATE 6000

// serial port for wind connector
#define SERIALSDS011 Serial1

// activate debug on serial port
#define DEBUGONSERIAL

#ifdef DEBUGONSERIAL
#define IF_SDEBUG(x) ({x;})
#else
#define IF_SDEBUG(x)
#endif

#define LEDPIN 13
#define FORCEDEFAULTPIN 8

// define the version of the configuration saved on eeprom
// if you chenge this the board start with default configuration at boot
#define CONFVER "confsd00"

#define SDS011PRESENT 1
#define MICS4514PRESENT 1

#define SCALE1PIN 8
#define SCALE2PIN 9

#define COPIN 4
#define NO2PIN 5
#define HEATERPIN 6
