///////////////////////////////////////////////////////////////////////////////////////////////////
// I2C SDS011 registers
///////////////////////////////////////////////////////////////////////////////////////////////////

#define I2C_SDS011_DEFAULTADDRESS        36                      //7 bit address

// all bit to 1 => 0xFFFF or 65535 for int 
#define MISSINTVALUE 0xFFFF

// offset to write signed int in unsigned int
#define OFFSET 32768


//
#define I2C_SDS011_COMMAND               0xFF
#define I2C_SDS011_COMMAND_ONESHOT_START    1
#define I2C_SDS011_COMMAND_ONESHOT_STOP     2
#define I2C_SDS011_COMMAND_STOP             3
#define I2C_SDS011_COMMAND_SAVE             4

#define I2C_SDS011_VERSION               0x00      // Version

#define I2C_SDS011_PM25                  0x01      // pm 2.5
#define I2C_SDS011_PM10                  0x03      // pm 10
#define I2C_SDS011_MINPM10               0x05      // pm 2.5 mean 
#define I2C_SDS011_MINPM10               0x07      // pm 10  mean
#define I2C_SDS011_MEANPM25              0x09      // pm 2.5 min
#define I2C_SDS011_MEANM10               0x0B      // pm 10  min
#define I2C_SDS011_MAXPM25               0x0D      // pm 2.5 max
#define I2C_SDS011_MAXPM10               0x0F      // pm 10  max
#define I2C_SDS011_SIGMAPM25             0x11      // pm 2.5 sigma
#define I2C_SDS011_SIGMAPM10             0x13      // pm 10  sigma

#define I2C_SDS011_CO                    0x15      // pm CO
#define I2C_SDS011_NO2                   0x17      // pm NO2
#define I2C_SDS011_MINCO                 0x19      // pm CO  mean 
#define I2C_SDS011_MINNO2                0x1B      // pm NO2 mean
#define I2C_SDS011_MEANCO                0x1D      // pm CO  min
#define I2C_SDS011_MEANNO2               0x1F      // pm NO2 min
#define I2C_SDS011_MAXCO                 0x21      // pm CO  max
#define I2C_SDS011_MAXNO2                0x23      // pm NO2 max
#define I2C_SDS011_SIGMACO               0x25      // pm CO  sigma
#define I2C_SDS011_SIGMANO2              0x27      // pm NO2 sigma


#define I2C_SDS011_MAP_WRITABLE          0x1F
#define I2C_SDS011_ONESHOT               0x1F      // saple mode (bool)
#define I2C_SDS011_ADDRESS               0x20      // i2c bus address (short unsigned int)


///////////////////////////////////////////////////////////////////////////////////////////////////
// End register definition 
///////////////////////////////////////////////////////////////////////////////////////////////////
