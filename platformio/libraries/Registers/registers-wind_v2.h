///////////////////////////////////////////////////////////////////////////////////////////////////
// I2C WIND registers
///////////////////////////////////////////////////////////////////////////////////////////////////

#define I2C_WIND_DEFAULTADDRESS        34                      //7 bit address

// all bit to 1 => 0xFFFF or 65535 for int 
#define MISSINTVALUE 0xFFFF

// offset to write signed int in unsigned int
#define OFFSET 32768

// sensortype table
#define DAVISSENSORTYPE 1
#define INSPEEDSENSORTYPE 2

//
#define I2C_WIND_COMMAND               0xFF
#define I2C_WIND_COMMAND_ONESHOT_START   1
#define I2C_WIND_COMMAND_ONESHOT_STOP    2
#define I2C_WIND_COMMAND_STOP            3
#define I2C_WIND_COMMAND_SAVE            4

#define I2C_WIND_VERSION               0x00      // Version
#define I2C_WIND_DD                    0x01      // DD
#define I2C_WIND_FF                    0x03      // FF
#define I2C_WIND_U                     0x05      // u component
#define I2C_WIND_V                     0x07      // v component
#define I2C_WIND_MEANU                 0x09      // 10 min mean U
#define I2C_WIND_MEANV                 0x0B      // 10 min mean V
#define I2C_WIND_PEAKGUSTU             0x0D      // peak gust U
#define I2C_WIND_PEAKGUSTV             0x0F      // peak gust V
#define I2C_WIND_LONGGUSTU             0x11      // long (60s) gust U
#define I2C_WIND_LONGGUSTV             0x13      // long (60s) gust V
#define I2C_WIND_MEANFF                0x15      // mean FF
#define I2C_WIND_SIGMA                 0x17      // sigma FF
#define I2C_WIND_SECTOR1               0x17      // frequency on sector 1
#define I2C_WIND_SECTOR2               0x17      // frequency on sector 2
#define I2C_WIND_SECTOR3               0x17      // frequency on sector 3
#define I2C_WIND_SECTOR4               0x17      // frequency on sector 4
#define I2C_WIND_SECTOR5               0x17      // frequency on sector 5
#define I2C_WIND_SECTOR6               0x17      // frequency on sector 6
#define I2C_WIND_SECTOR7               0x17      // frequency on sector 7
#define I2C_WIND_SECTOR8               0x17      // frequency on sector 8
#define I2C_WIND_SECTORCALM            0x19      // frequency of calm wind

#define I2C_WIND_MAP_WRITABLE          0x1F
#define I2C_WIND_ONESHOT               0x1F      // sample mode (bool)
#define I2C_WIND_ADDRESS               0x20      // i2c bus address (short unsigned int)
#define I2C_WIND_SENSORTYPE            0x21      // sensor type table (short unsigned int)


///////////////////////////////////////////////////////////////////////////////////////////////////
// End register definition 
///////////////////////////////////////////////////////////////////////////////////////////////////
