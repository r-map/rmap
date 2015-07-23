//////////////////////////////////////////////////////////////////////////////
// i2C comm definitions
//

// we use interrupt 0 so un arduino uno digital pin 2 is used for wind intensiti switch sensor

// this pin is connected to middle potenziometer for direction
#define ANALOGPIN 0


// activate debug on serial port
#define DEBUGONSERIAL

#ifdef DEBUGONSERIAL
#define IF_SDEBUG(x) ({x;})
#else
#define IF_SDEBUG(x)
#endif

