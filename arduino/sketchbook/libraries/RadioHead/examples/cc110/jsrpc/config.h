# define VERSION "220171127"

// radio bidirectional comunication
//#define TWOWAY "Yes"
#define CLIENT "Yes"
//#define SERVER "Yes"


// define the  pins used
#define PINS 4,5,A6,A7

#define SERIALBUFFERSIZE 120
#define SERIALBAUDRATE 115200
//#define DEBUGONSERIAL


#ifdef DEBUGONSERIAL

#define DBGSERIAL Serial

#define IF_SDEBUG(x) ({x;})
#else
#define IF_SDEBUG(x)
#endif
