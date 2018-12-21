//disable debug at compile time but call function anyway
//#define DISABLE_LOGGING true

// set the I2C clock frequency 
#define I2C_CLOCK 30418

// define the version of the configuration saved on eeprom
// if you change this the board start with default configuration at boot
// var CONFVER max lenght 10 char!
#define CONFVER "confgpi00"

// number of analog sample to do for average
#define NSAMPLE 6

// stepper default parameter
#define STEPPER_POWER 1023
#define STEPPER_SPEED 4096
#define STEPPER_RAMPSTEPS 400
#define STEPPER_HALFSTEP true

// button parameter
// this activate button as default configuration
// when button is active sleep is disables so more power is needed
#define BUTTONACTIVEFORDEFAULT true
// dbTime: Debounce time in milliseconds. Defaults  25ms (unsigned long)
#define BUTTONDBTIME 25
// puEnable: true to enable the microcontroller's internal pull-up resistor, else false. Defaults to true. (bool)
#define BUTTONPUENABLE  true
// invert: false interprets a high logic level to mean the button is pressed, true interprets a low level as pressed. true should be used when a pull-up resistor is employed, false for a pull-down resistor. Defaults to true. (bool)
#define BUTTONINVERT true
// we define a "long press" time in milliseconds
#define BUTTONLONG_PRESS 1000

// pins definitions
#define LED_PIN  13
#define CHANGEADDRESS1 6     // add 1 to i2c address
#define CHANGEADDRESS2 7     // add 2 to i2c address
#define FORCEDEFAULTPIN 8
#define PWM1_PIN  13
#define PWM2_PIN  2
#define ONOFF1_PIN  4
#define ONOFF2_PIN  5
#define ANALOG1_PIN  0
#define ANALOG2_PIN  1
#define STEPPER_PIN1 12
#define STEPPER_PIN2 10
#define STEPPER_PIN3 11
#define STEPPER_PIN4 9
#define BUTTON1PIN   3
#define ENCODERA CHANGEADDRESS1
#define ENCODERB CHANGEADDRESS2


