//disable debug at compile time but call function anyway
//#define DISABLE_LOGGING true

// set the I2C clock frequency 
#define I2C_CLOCK 30418

// define the version of the configuration saved on eeprom
// if you change this the board start with default configuration at boot
// var CONFVER max lenght 10 char!
#define CONFVER "confmng00"

// pins definitions
#define LED_PIN  D4
#define CHANGEADDRESS1 D6     // add 1 to i2c address
#define CHANGEADDRESS2 D7     // add 2 to i2c address
#define FORCEDEFAULTPIN D8

