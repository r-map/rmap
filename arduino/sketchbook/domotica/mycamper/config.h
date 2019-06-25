//#define APMODE 1

//#define I2CPULLUP Yes

#define RECV_PIN  D5    // Define IR sensor pin
#define SCL D1
#define SDA D2
#define RESET_PIN D3    // pin to connect to ground for reset wifi configuration
#define LED_PIN D4

#define NPINOUT 3
const uint8_t GPIOPIN[NPINOUT] = {D0,D6,D7};  // output pins

#define NI2CGPIOPIN 2
const uint8_t I2CGPIOPIN[NI2CGPIOPIN] = {1,2};  // i2c output pins

#define WIFI_APMODE 1
const char* ota_server= "rmap.cc";
#define WIFI_SSED "mycamper"
#define WIFI_PASSWORD  "ford1234"
#define OLEDI2CADDRESS 0X3C

// set the frequency
#define I2C_CLOCK 10000

// logging level at compile time
// Available levels are:
// LOG_LEVEL_SILENT, LOG_LEVEL_FATAL, LOG_LEVEL_ERROR, LOG_LEVEL_WARNING, LOG_LEVEL_NOTICE, LOG_LEVEL_VERBOSE
#define LOG_LEVEL   LOG_LEVEL_NOTICE

// time to stay on before to go in sleep mode in seconds
#define TIMEON  30

// delay on display updates
#define TIMEDISPLAY  1

// IR telecontrol CODE
#define DECODETYPE NEC
#define KEYPAD_0     0xFF48B7 // 0 Keypad Button
#define KEYPAD_1     0xFF906F // 1 Keypad Button
#define KEYPAD_2     0xFFB847 // 2 Keypad Button
#define KEYPAD_3     0xFFF807 // 3 Keypad Button
#define KEYPAD_4     0xFFB04F // 4 Keypad Button
#define KEYPAD_5     0xFF9867 // 5 Keypad Button
#define KEYPAD_6     0xFFD827 // 6 Keypad Button
#define KEYPAD_7     0xFF8877 // 7 Keypad Button
#define KEYPAD_8     0xFFA857 // 8 Keypad Button
#define KEYPAD_9     0xFFE817 // 9 Keypad Button
#define KEYPAD_MINUS 0xFF50AF // Vol- Keypad Button
#define KEYPAD_PLUS  0xFF7887 // Vol+ Keypad Button
#define KEYPAD_DOWN  0xFF40BF // CH- Keypad Button
#define KEYPAD_UP    0xFFA05F // CH+ Keypad Button
#define KEYPAD_OK    0xFF02FD // full screen Keypad Button
#define KEYPAD_POWERDOWN 0xFFB24D // powerdown Keypad Button

#define MINUS 1
#define PLUS  2
#define DOWN  3
#define UP    4
#define OK    5
