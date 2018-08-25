// activate debug on serial port
#define DEBUGONSERIAL

#ifdef DEBUGONSERIAL
#define IF_SDEBUG(x) ({x;})
#else
#define IF_SDEBUG(x)
#endif

// set the I2C clock frequency 
#define I2C_CLOCK 30418

#define LEDPIN 13
#define FORCEDEFAULTPIN 8

// define the version of the configuration saved on eeprom
// if you chenge this the board start with default configuration at boot
#define CONFVER "confpwm00"

// pins definitions
#define LED_PIN  13

#define PWM1_PIN  2
#define PWM2_PIN  3
