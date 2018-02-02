// activate debug on serial port
//#define SDDEBUGONSERIAL


// add secondary to primary parameters to send in multiparameter sensors 
//#define SECONDARYPARAMETER

//those ms after a prepare the measure will be too old to be considered valid

// this the value for mqtt and sample/observation
//#define MAXDELAYFORREAD 8000

// this the value for http and report
#define MAXDELAYFORREAD 60000

// add getdata method in library for lora-ttn compression
#define USEGETDATA

// use ajson library for json response
#define USEAJSON

// use aarduinojson library for json response
//#define USEARDUINOJSON

// use RF24Network library for radio transport
//#define RADIORF24

// use AES library for radio transport
//#define AES

// retry number for multimaster I2C configuration
#define NTRY 3 

// include TMP driver
//#define TMPDRIVER

// include ADT driver
#define ADTDRIVER

// include HIH driver
#define HIHDRIVER

// include HYT driver
//#define HYTDRIVER

// include BMP driver
//#define BMPDRIVER
//#define BMP085_DEBUG 1

//include SI7021
//#define HI7021DRIVER

// include DAVIS WIND driver
//#define DAVISWIND1

// include tipping bucket rain gauge driver
//#define TIPPINGBUCKETRAINGAUGE

// include TH temperature/humidity driver SAMPLE MODE
//#define TEMPERATUREHUMIDITY_ONESHOT

// include TH temperature/humidity driver REPORT MODE
//#define TEMPERATUREHUMIDITY_REPORT

// include sds011 pm 2.5 and pm 10 driver SAMPLE MODE
//#define SDS011_ONESHOT
//luftdaten
//#define SDS_PIN_RX D1
//#define SDS_PIN_TX D2
#define SDS_PIN_RX D5
#define SDS_PIN_TX D6
#define SDSSAMPLES 3

// include sds011 pm 2.5 and pm 10 driver REPORT MODE
//#define SDS011_REPORT

// include mics4514 CO and NO2pm driver SAMPLE MODE
#define MICS4514_ONESHOT

// include mics4514 CO and NO2pm driver REPORT MODE
//#define MICS4514_REPORT


#if defined (TEMPERATUREHUMIDITY_ONESHOT)
#if defined (TEMPERATUREHUMIDITY_REPORT)
CANNOT DEFINE TEMPERATUREHUMIDITY_ONESHOT AND TEMPERATUREHUMIDITY_REPORT TOGETHER
#endif
#endif

#if defined (SDS011_ONESHOT)
#if defined (SDS011_REPORT)
CANNOT DEFINE SDS011_ONESHOT AND SDS011_REPORT TOGETHER
#endif
#endif

#if defined (MICS4514_ONESHOT)
#if defined (MICS4514_REPORT)
CANNOT DEFINE MICS4514_ONESHOT AND MICS4514_REPORT TOGETHER
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
