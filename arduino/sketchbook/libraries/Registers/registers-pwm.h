///////////////////////////////////////////////////////////////////////////////////////////////////
// I2C TH registers
///////////////////////////////////////////////////////////////////////////////////////////////////

#define I2C_PWM_DEFAULTADDRESS         8                      //7 bit address 0x40 write, 0x41 read

// all bit to 1 => 0xFFFF or 65535 for int 
#define MISSINTVALUE 0xFFFF

// offset to write signed int in unsigned int
#define OFFSET 32768


//
#define I2C_PWM_COMMAND               0xFF
#define I2C_PWM_COMMAND_DO              1
#define I2C_PWM_COMMAND_ONESHOT_START   2
#define I2C_PWM_COMMAND_ONESHOT_STOP    3
#define I2C_PWM_COMMAND_SAVE            4

#define I2C_PWM_VERSION               0x00      // Version		   
#define I2C_PWM_ANALOG1               0x01      // analog value1
#define I2C_PWM_ANALOG2               0x03      // analog value2

#define I2C_PWM_MAP_WRITABLE          0x1F
#define I2C_PWM_ONESHOT               0x1F      // sample mode (bool)
#define I2C_PWM_ADDRESS               0x20      // i2c bus address (short unsigned int)
#define I2C_PWM_PWM1                  0x21      // pwm value
#define I2C_PWM_PWM2                  0x22      // pwm value
#define I2C_PWM_ONOFF1                0x23      // on/off value
#define I2C_PWM_ONOFF2                0x24      // on/off value


///////////////////////////////////////////////////////////////////////////////////////////////////
// End register definition 
///////////////////////////////////////////////////////////////////////////////////////////////////
