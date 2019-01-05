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

// set software pullup for encoder input A & B 
#define ENCODERPULLUP true

// button parameter
// this activate button as default configuration
// when button is active sleep is disables so more power is needed
#define BUTTONACTIVEFORDEFAULT true
// dbTime: Debounce time in milliseconds. Defaults  25ms (unsigned long)
#define BUTTONDBTIME 25
// puEnable: true to enable the microcontroller's internal pull-up
// resistor, else false. Defaults to true. (bool)
#define BUTTONPUENABLE  true
// invert: false interprets a high logic level to mean the button is
// pressed, true interprets a low level as pressed. true should be
// used when a pull-up resistor is employed, false for a pull-down
// resistor. Defaults to true. (bool)
#define BUTTONINVERT true
// we define a "long press" time in milliseconds
#define BUTTONLONG_PRESS 1000

/*
Digital I/O There are 22 digital input/output ports totally:
    They are labeled on the module of D0, D1, D2~D13, and A0~A7, of which A6 and A7 can only input.
Analog I/O There are 8 analog input ports totally:
    They are labeled on the module of A0~A7;
PWM supports, a total of 6:
    Respectively labeled on the module of D3,D5,D6,D9,D10,D11.
A serial port support, a total of 1:
    Labeled on the module of Serial[D0(RX), D1(TX)]
SPI support, a total of 1:
    Labeled on the module of D13(SCK), D12(MISO), D11(MOSI), D10(SS).
I2C support, a total of 1:
    Labeled on the module of SDA(A4), SCL(A5).
External interrupt support, a total of 2:
    Labeled on the module of D2(interrupt0), D3(interrupt1).
*/

// pins definitions
//                      0     // RX
//                      1     // TX
#define BUTTON1PIN      2     // input button 1 (INT0 not used)
#define PWM1_PIN        3     // output PWM 1
#define STEPPER_PIN1    4     // output stepper 1
#define ONOFF1_PIN      5     // output on/off 1
#define ONOFF2_PIN      6     // output on/off 2
#define STEPPER_PIN2    7     // output stepper 2
#define FORCEDEFAULTPIN 8     // input force default
//                      9     // output stepper PWM 1
//                      10    // output stepper PWM 2
#define PWM2_PIN        11    // output PWM 2
#define STEPPER_PIN3    12    // output stepper 3
#define STEPPER_PIN4    13    // output stepper 4
#define ANALOG1_PIN     A0     // D14  analog input 1
#define ANALOG2_PIN     A1     // D15  analog input 2
#define ENCODERA        16    // input decoder A
#define ENCODERB        17    // input decoder B
//                      18    // SDA
//                      19    // SCL
#define CHANGEADDRESS1  20    // input add 1 to i2c address  
#define CHANGEADDRESS2  21    // input add 2 to i2c address

