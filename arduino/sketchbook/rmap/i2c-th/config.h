//sensor definition

#define TEMPERATURE_ADDRESS 73
#define HUMIDITY_ADDRESS 39

#define SAMPLERATE 3000
#define MINUTEFORREPORT 2

// max missing data for minute
#define MAXMISSING 1

// activate debug on serial port
//#define DEBUGONSERIAL

#ifdef DEBUGONSERIAL
#define IF_SDEBUG(x) ({x;})
#else
#define IF_SDEBUG(x)
#endif

// set the I2C clock frequency 
#define I2C_CLOCK 30418
