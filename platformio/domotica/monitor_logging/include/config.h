//#define DISABLE_LOGGING 
//#define EXTERNALRTC

#define PINGTIMEOUT   10000      // millisec
#define HEARTBEATTIME 60         // seconds
#define SIZEFILEMAX   100000     // byte 

#ifdef LED_BUILTIN
#define LEDPIN        LED_BUILTIN
#else
#define LEDPIN        D13     // PB3
#endif

#define SERIAL1RX     PA10    // D0
#define SERIAL1TX     PA9     // D1

// I2C   SDA A4       PA5
// I2C   SCL A5       PA6

// change this to match your SD shield or module;
// Default SPI on Uno and Nano: pin 10
// Arduino Ethernet shield: pin 4
// Adafruit SD shields and modules: pin 10
// Sparkfun SD shield: pin 8
// MKRZero SD: SDCARD_SS_PIN
#define SDSELECT      D10
#define CARDDETECT    A1
#define BUZZERPIN     D9

// rotary encoder pins
#define encBtn        D6
#define encA          D2
#define encB          D3

#define OLEDI2CADDRESS 0X3C
