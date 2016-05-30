//sensor definition

#define TEMPERATURE_ADDRESS 0x33
#define HUMIDITY_ADDRESS 0x39

#define SAMPLERATE 3000


// activate debug on serial port
#define DEBUGONSERIAL

#ifdef DEBUGONSERIAL
#define IF_SDEBUG(x) ({x;})
#else
#define IF_SDEBUG(x)
#endif

