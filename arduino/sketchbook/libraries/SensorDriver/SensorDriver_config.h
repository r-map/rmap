// activate debug on serial port
#define SDDEBUGONSERIAL

//those ms after a prepare the measure will be too old to be considered valid
#define MAXDELAYFORREAD 8000

// use ajson library for json response
#define USEAJSON

// use RF24Network library for radio transport
//#define RADIORF24

// use AES library for radio transport
//#define AES

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

// include TH temperature/humidity driver
#define TEMPERATUREHUMIDITY

#if defined (TIPPINGBUCKETRAINGAUGE)
 // how many rain for one tick of the rain gauge (Hg/m^2)
 // 0.2 Kg/m^2 for tips
 #define RAINFORTIP 2    
#endif
