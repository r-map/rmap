// wind sensor definition

//#define DAVIS
#define WINDSONIC

// other parameters

#if defined(WINDSONIC)
// time in us equired for oneshot measure
// setting windsonic for 4 sample for second we wait a little more
#define SAMPLETIME 300
#endif

//// take one measure every SAMPLERATE us
// setting windsonic for 4 sample for second we wait double for non oneshot measuremets
#define SAMPLERATE 500

// serial port for wind connector
#define SERIALWIND Serial1

// activate debug on serial port
#define DEBUGONSERIAL

#ifdef DEBUGONSERIAL
#define IF_SDEBUG(x) ({x;})
#else
#define IF_SDEBUG(x)
#endif

