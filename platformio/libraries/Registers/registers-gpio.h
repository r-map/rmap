///////////////////////////////////////////////////////////////////////////////////////////////////
// I2C TH registers
///////////////////////////////////////////////////////////////////////////////////////////////////

#define I2C_GPIO_DEFAULTADDRESS         5                      //7 bit address 0x40 write, 0x41 read

// all bit to 1 => 0xFFFF or 65535 for int 
#define MISSINTVALUE 0xFFFF

// offset to write signed int in unsigned int
#define OFFSET 32768


// registers
#define I2C_GPIO_COMMAND                         0xFF
#define I2C_GPIO_COMMAND_TAKE                    1
#define I2C_GPIO_COMMAND_ONESHOT_START           2
#define I2C_GPIO_COMMAND_ONESHOT_STOP            3
#define I2C_GPIO_COMMAND_SAVE                    4
#define I2C_GPIO_STEPPER_COMMAND_GOTO		 5
#define I2C_GPIO_STEPPER_COMMAND_READ_POSITION	 6
#define I2C_GPIO_STEPPER_COMMAND_POWEROFF	 7
#define I2C_GPIO_STEPPER_COMMAND_RELATIVE_STEPS	 8
#define I2C_GPIO_STEPPER_COMMAND_ROTATE	         9
#define I2C_GPIO_STEPPER_COMMAND_GOHOME	        10
#define I2C_GPIO_SERVO_COMMAND_GOTO		11

#define I2C_GPIO_VERSION                      0x00      // Version		   
#define I2C_GPIO_ANALOG1                      0x01      // analog value1
#define I2C_GPIO_ANALOG2                      0x03      // analog value2
#define I2C_GPIO_STEPPER_CURRENT_POSITION     0x05
#define I2C_GPIO_LAST_COMMAND                 0x07

#define I2C_GPIO_MAP_WRITABLE                 0x1F
#define I2C_GPIO_ONESHOT                      0x1F      // sample mode (bool)
#define I2C_GPIO_ADDRESS                      0x20      // i2c bus address (short unsigned int)
#define I2C_GPIO_PWM1                         0x21      // pwm value
#define I2C_GPIO_PWM2                         0x22      // pwm value
#define I2C_GPIO_ONOFF1                       0x23      // on/off value
#define I2C_GPIO_ONOFF2                       0x24      // on/off value
#define I2C_GPIO_STEPPER_GOTO_POSITION        0x25      // 16 bits
#define I2C_GPIO_STEPPER_SPEED                0x27      // 16 bits
#define I2C_GPIO_STEPPER_TORQUE               0x29      // 16 bits
#define I2C_GPIO_STEPPER_RAMP_STEPS           0x2B      // 16 bits
#define I2C_GPIO_STEPPER_MODE                 0x2D      // 16 bits
#define I2C_GPIO_STEPPER_RELATIVE_STEPS       0x2F      // 16 bits
#define I2C_GPIO_STEPPER_ROTATE_DIR           0x31      // 16 bits
#define I2C_GPIO_STEPPER_POWER                0x33      // 16 bits
#define I2C_GPIO_STEPPER_HALFSTEP             0x35      // bool
#define I2C_GPIO_BUTTON_ACTIVE                0x36      // bool
#define I2C_GPIO_BUTTON_LONG_TIME             0x37      // 16 bits
#define I2C_GPIO_IRREMOTE_MODEL               0x39      // 8 bits
#define I2C_GPIO_IRREMOTE_CODE                0x3A      // 32 bits
#define I2C_GPIO_IRREMOTE_BIT                 0x3E      // 8 bits
#define I2C_GPIO_IRREMOTE_REPEAT              0x3F      // 8 bits
#define I2C_GPIO_SERVO1_GOTO_POSITION         0x40      // 16 bits
#define I2C_GPIO_SERVO2_GOTO_POSITION         0x42      // 16 bits

///////////////////////////////////////////////////////////////////////////////////////////////////
// End register definition 
///////////////////////////////////////////////////////////////////////////////////////////////////
