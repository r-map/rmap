// activate debug on serial port
//#define CADEBUGONSERIAL

#define MAX_POINTS	5	// MAX calibration points


#ifdef CADEBUGONSERIAL

#define CADBGSERIAL Serial

#define IF_CASDEBUG(x) ({x;})
#else
#define IF_CASDEBUG(x)
#endif
