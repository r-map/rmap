//disable debug at compile time but call function anyway
//#define DISABLE_LOGGING true

// logging level at compile time
// Available levels are:
// LOG_LEVEL_SILENT, LOG_LEVEL_FATAL, LOG_LEVEL_ERROR, LOG_LEVEL_WARNING, LOG_LEVEL_NOTICE, LOG_LEVEL_VERBOSE
#define LOG_LEVEL   LOG_LEVEL_NOTICE


// set the I2C clock frequency 
#define I2C_CLOCK 30418

// define the version of the configuration saved on eeprom
// if you change this the board start with default configuration at boot
// var CONFVER max lenght 10 char!
#define CONFVER "confgpi00"

// define if you want to use IRremote as alternative to PWM1 and PWM2 output
//#define IRREMOTE

// define if you want to use two servo motors  as alternative to one STEPPER motor
//#define SERVO

#define SERVO1_MIN 930                  // servo 1 Model SM-S2309S   (0 - 120 degree)
#define SERVO1_MAX 3000
#define SERVO2_MIN 544                  // servo 2 default parameters
#define SERVO2_MAX 2400

// number of analog sample to do for average
#define NSAMPLE 6

// stepper default parameter
#define STEPPER_POWER 1023
#define STEPPER_SPEED 4096
#define STEPPER_RAMPSTEPS 400
#define STEPPER_HALFSTEP false

// use i2c in multimaster mode (encoder and button push mode)
//#define MULTIMASTER 

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
#ifdef IRREMOTE
//#define               3     // output IR transmitter hardware defined in lib
#else
#define PWM1_PIN        3     // output PWM 1
#endif
#define STEPPER_PIN1    4     // output stepper 1
#define ONOFF1_PIN      5     // output on/off 1
#define ONOFF2_PIN      6     // output on/off 2
#define STEPPER_PIN2    7     // output stepper 2
#define FORCEDEFAULTPIN 8     // input force default
#ifdef SERVO
  #define SERVO1_PIN    9     // output PWM for servo
  #define SERVO2_PIN    10    // output PWM for servo
#else
  //                    9     // output stepper PWM 1
  //                    10    // output stepper PWM 2
#endif
#ifdef IRREMOTE
  #define RECV_PIN      11    // input IR receiver
#else
  #define PWM2_PIN      11    // output PWM 2
#endif
#define STEPPER_PIN3    12    // output stepper 3
#define STEPPER_PIN4    13    // output stepper 4
#define CHANGEADDRESS1  14    // input add 1 to i2c address  
#define CHANGEADDRESS2  15    // input add 2 to i2c address
#define ENCODERA        16    // input decoder A
#define ENCODERB        17    // input decoder B
//                      18    // SDA
//                      19    // SCL
#define ANALOG1_PIN     A6    // D20  analog input 1  (only input)
#define ANALOG2_PIN     A7    // D21  analog input 2  (only input)


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

#define IR_SONY      1  // send type for IR
#define IR_PANASONIC 2  // send type for IR

#define MINUS 10
#define PLUS  11
#define DOWN  12
#define UP    13
#define OK    14
#define OKKAY 15
