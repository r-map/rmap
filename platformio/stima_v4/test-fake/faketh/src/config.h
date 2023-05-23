
#define LEDPIN 13
#define FORCEDEFAULTPIN 8

// define the version of the configuration saved on eeprom
// if you chenge this the board start with default configuration at boot
#define CONFVER "confth00"

// activate debug on serial port as alternative to plot graph
#define DEBUGONSERIAL

#ifdef DEBUGONSERIAL
#define IF_SDEBUG(x) ({x;})
#else
#define IF_SDEBUG(x)
#define PLOT
#endif
