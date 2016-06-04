//sensor definition

#define TEMPERATURE_ADDRESS 73
#define HUMIDITY_ADDRESS 39

#define SAMPLERATE 3000
#define MINUTEFORREPORT 1

// activate debug on serial port
//#define DEBUGONSERIAL

#ifdef DEBUGONSERIAL
#define IF_SDEBUG(x) ({x;})
#else
#define IF_SDEBUG(x)
#endif
