//////////////////////////////////////////////////////////////////////////////
// i2C comm definitions
//

// we use interrupt 0 so un arduino uno digital pin 2 is used for wind intensiti switch sensor

// this pin is connected to switch of rain gauge
#define RAINGAUGEPIN 2

//Bouncing is the tendency of any two metal contacts in an electronic
//device to generate multiple signals as the contacts close or
//open. There's a minimum delay between toggles to debounce the
//circuit (i.e. to ignore noise).
// millisec

// with 200 the maximum rain rate is 1 Kg/m^2/s with a  tipping bucket rain gauge with .2 Kg/m^2 tips
#define DEBOUNCINGTIME 200

// temporary patch for microduino; see http://forum.microduino.cc/topic/91/digitalpintointerrupt-was-not-declared-in-this-scope
#define digitalPinToInterrupt(p) ((p) == 2 ? 0 : ((p) == 3 ? 1 : ((p) >= 18 && (p) <= 21 ? 23 - (p) : NOT_AN_INTERRUPT)))


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
#define CONFVER "confra00"

