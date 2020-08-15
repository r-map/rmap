# define VERSION "20200814"

// radio bidirectional comunication
//#define TWOWAY "Yes"
//#define CLIENT "Yes"
//#define SERVER "Yes"

// for bluepill
//#define JSSERIAL SerialUSB

// other board
//#define JSSERIAL Serial

// freq added to standard channel
//#define FREQCORR 0.050
#define FREQCORR 0.0

// define the  pins used
#ifdef ARDUINO_ARCH_AVR
#define PINS 4,5,A6,A7
#else
#define PINS 22,23,24,25
#endif

#define SERIALBUFFERSIZE 160
#define SERIALBAUDRATE 115200
#define DEBUGONSERIAL


#ifdef DEBUGONSERIAL

//#define DBGSERIAL SerialUSB

#define IF_SDEBUG(x) ({x;})
#else
#define IF_SDEBUG(x)
#endif
