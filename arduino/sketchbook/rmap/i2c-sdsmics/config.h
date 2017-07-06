// sds011 sensor definition

//// take one measure every SAMPLERATE us
#define SAMPLERATE 6000

// serial port for wind connector
#define SERIALSDS011 Serial1

// activate debug on serial port
#define DEBUGONSERIAL

#ifdef DEBUGONSERIAL
#define IF_SDEBUG(x) ({x;})
#else
#define IF_SDEBUG(x)
#endif

#define LEDPIN 13
#define FORCEDEFAULTPIN 8

// define the version of the configuration saved on eeprom
// if you chenge this the board start with default configuration at boot
#define CONFVER "confsd01"

#define SDS011PRESENT 1
#define MICS4514PRESENT 1

#define SCALE1PIN 8
#define SCALE2PIN 9

#define COPIN 1
#define NO2PIN 0
#define HEATERPIN 7


// default calibration data

// Concentrations used in calibration process
#define POINT1_PPM_NO2 10.0   // <-- Normal concentration in air
#define POINT2_PPM_NO2 50.0   
#define POINT3_PPM_NO2 100.0 

// Calibration resistences obtained during calibration process (in KOHMs)
#define POINT1_RES_NO2 45.25  // <-- Rs at normal concentration in air
#define POINT2_RES_NO2 25.50
#define POINT3_RES_NO2 3.55

// Define the number of calibration points
#define NO2NUMPOINTS 3

// Concentratios used in calibration process
#define POINT1_PPM_CO 100.0   // <--- Ro value at this concentration
#define POINT2_PPM_CO 300.0   // 
#define POINT3_PPM_CO 1000.0  // 

// Calibration resistances obtained during calibration process
#define POINT1_RES_CO 230.30 // <-- Ro Resistance at 100 ppm. Necessary value.
#define POINT2_RES_CO 40.665 //
#define POINT3_RES_CO 20.300 //

// Define the number of calibration points
#define CONUMPOINTS 3


//https://uk-air.defra.gov.uk/assets/documents/reports/cat06/0502160851_Conversion_Factors_Between_ppb_and.pdf
#define NO2PPM2UGM3 1.9125
#define COPPM2UGM3  1.1642
