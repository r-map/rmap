// wind sensor definition

//#define DAVIS
#define WINDSONIC

// other parameters

#if defined(WINDSONIC)
// time in us equired for oneshot measure
#define SAMPLETIME 300
#endif

// take one measure every SAMPLERATE us
#define SAMPLERATE 1000

// serial port for wind connector
#define SERIALWIND Serial1

// activate debug on serial port
#define DEBUGONSERIAL

#ifdef DEBUGONSERIAL
#define IF_SDEBUG(x) ({x;})
#else
#define IF_SDEBUG(x)
#endif

