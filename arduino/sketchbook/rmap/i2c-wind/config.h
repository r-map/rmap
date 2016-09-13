// wind sensor default definition
#define SENSORTYPE DAVISSENSORTYPE
//#define SENSORTYPE INSPEEDSENSORTYPE

// in non oneshot mode take one measure every SAMPLERATE us
#define SAMPLERATE 3000


//////////////////////////////////////////////////////////////////////////////
// i2C comm definitions
//

// we use interrupt 0 so in arduino uno digital pin 2 is used for wind intensity switch sensor
#define INTERRUPTPIN 2
// temporary patch for microduino; see http://forum.microduino.cc/topic/91/digitalpintointerrupt-was-not-declared-in-this-scope
#define digitalPinToInterrupt(p) ((p) == 2 ? 0 : ((p) == 3 ? 1 : ((p) >= 18 && (p) <= 21 ? 23 - (p) : NOT_AN_INTERRUPT)))

// this pin is connected to middle pin of potenziometer for direction
#define ANALOGPIN 0


// activate debug on serial port
//#define DEBUGONSERIAL

#ifdef DEBUGONSERIAL
#define IF_SDEBUG(x) ({x;})
#else
#define IF_SDEBUG(x)
#endif

#define LEDPIN 13
#define FORCEDEFAULTPIN 8

// define the version of the configuration saved on eeprom
// if you chenge this the board start with default configuration at boot
#define CONFVER "conf00"

