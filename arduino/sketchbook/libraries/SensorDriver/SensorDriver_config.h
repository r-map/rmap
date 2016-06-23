// activate debug on serial port
//#define SDDEBUGONSERIAL

//those ms after a prepare the measure will be too old to be considered valid

// this the value for mqtt and sample/observation
//#define MAXDELAYFORREAD 8000

// this the value for http and report
#define MAXDELAYFORREAD 60000

// use ajson library for json response
#define USEAJSON

// use RF24Network library for radio transport
//#define RADIORF24

// use AES library for radio transport
//#define AES

// retry number for multimaster I2C configuration
#define NTRY 3 

// include TMP driver
#define TMPDRIVER

// include ADT driver
#define ADTDRIVER

// include HIH driver
#define HIHDRIVER

// include BMP driver
#define BMPDRIVER
//#define BMP085_DEBUG 1

// include DAVIS WIND driver
#define DAVISWIND1

// include tipping bucket rain gauge driver
#define TIPPINGBUCKETRAINGAUGE

// include TH temperature/humidity driver SAMPLE MODE
//#define TEMPERATUREHUMIDITY_ONESHOT

// include TH temperature/humidity driver REPORT MODE
#define TEMPERATUREHUMIDITY_REPORT

#if defined (TEMPERATUREHUMIDITY_ONESHOT)
#if defined (TEMPERATUREHUMIDITY_REPORT)
CANNOT DEFINE TEMPERATUREHUMIDITY_ONESHOT AND TEMPERATUREHUMIDITY_REPORT TOGETHER
#endif
#endif

#if defined (TIPPINGBUCKETRAINGAUGE)
 // how many rain for one tick of the rain gauge (Hg/m^2)
 // 0.2 Kg/m^2 for tips
 #define RAINFORTIP 2    
#endif

#ifdef SDDEBUGONSERIAL

#define SDDBGSERIAL Serial

#define IF_SDSDEBUG(x) ({x;})
#else
#define IF_SDSDEBUG(x)
#endif
