//#define APMODE 1

//#define I2CPULLUP Yes

#define RECV_PIN  D5    // Define IR sensor pin
#define SCL D1
#define SDA D2
#define RESET_PIN D3    // pin to connect to ground for reset wifi configuration
#define LED_PIN D4
const uint8_t GPIOPIN[4] = {D0,D6,D7,D8};  // output pins

#define WIFI_APMODE 1
const char* ota_server= "rmap.cc";
#define WIFI_SSED "mycamper"
#define WIFI_PASSWORD  "ford1234"
#define OLEDI2CADDRESS 0X3C

// set the frequency
// 30418,25 Hz  : minimum freq with prescaler set to 1 and CPU clock to 16MHz 
#define I2C_CLOCK 30418

// logging level at compile time
// Available levels are:
// LOG_LEVEL_SILENT, LOG_LEVEL_FATAL, LOG_LEVEL_ERROR, LOG_LEVEL_WARNING, LOG_LEVEL_NOTICE, LOG_LEVEL_VERBOSE
#define LOG_LEVEL   LOG_LEVEL_NOTICE

// time to stay on before to go in sleep mode in seconds
#define TIMEON  30

// IR telecontrol CODE
#define DECODETYPE NEC
#define KEYPAD1 0xFF906F // 1 Keypad Button
#define KEYPAD2 0xFFB847 // 2 Keypad Button
#define KEYPAD3 0xFFF807 // 3 Keypad Button
#define KEYPAD4 0xFFB04F // 4 Keypad Button
