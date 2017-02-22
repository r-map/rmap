///////////////////////////////////////////////////////////////////////////////////////////////////
// I2C WIND registers
///////////////////////////////////////////////////////////////////////////////////////////////////

#define I2C_RAIN_DEFAULTADDRESS        0x21

// all bit to 1 => 0xFFFF or 65535 for int 
#define MISSINTVALUE 0xFFFF

// offset to write signed int in unsigned int
#define OFFSET 32768


//
#define I2C_RAIN_COMMAND               0xFF
#define I2C_RAIN_COMMAND_START           1
#define I2C_RAIN_COMMAND_STOP            2
#define I2C_RAIN_COMMAND_STARTSTOP       3
#define I2C_RAIN_COMMAND_SAVE            4

#define I2C_RAIN_VERSION               0x00      // Version
#define I2C_RAIN_TIPS                  0x01      // TIPS

#define I2C_RAIN_MAP_WRITABLE          0x1F
#define I2C_RAIN_ONESHOT               0x1F      // saple mode (bool)
#define I2C_RAIN_ADDRESS               0x21      // i2c bus address (short unsigned int)

///////////////////////////////////////////////////////////////////////////////////////////////////
// End register definition 
///////////////////////////////////////////////////////////////////////////////////////////////////
