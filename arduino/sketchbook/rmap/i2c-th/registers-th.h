///////////////////////////////////////////////////////////////////////////////////////////////////
// I2C TH registers
///////////////////////////////////////////////////////////////////////////////////////////////////

#define I2C_TH_DEFAULTADDRESS        35                      //7 bit address 0x40 write, 0x41 read

// all bit to 1 => 0xFFFF or 65535 for int 
#define MISSINTVALUE 0xFFFF

// offset to write signed int in unsigned int
#define OFFSET 32768


//
#define I2C_TH_COMMAND               0xFF
#define I2C_TH_COMMAND_ONESHOT_START   1
#define I2C_TH_COMMAND_ONESHOT_STOP    2
#define I2C_TH_COMMAND_START           3
#define I2C_TH_COMMAND_STOP            4
#define I2C_TH_COMMAND_STOP_START      5
#define I2C_TH_COMMAND_SAVE            6

#define I2C_TH_VERSION               0x00      // Version		   
#define I2C_TEMPERATURE_SAMPLE       0x01      // temperature sample	   
#define I2C_TEMPERATURE_MEAN60       0x03      // temperature mean 60 sec  
#define I2C_TEMPERATURE_MEAN         0x05      // temperature mean	   
#define I2C_TEMPERATURE_MAX          0x07      // temperature max	   
#define I2C_TEMPERATURE_MIN          0x09      // temperature min	   
#define I2C_TEMPERATURE_SIGMA        0x0B      // temperature sigma        

#define I2C_HUMIDITY_SAMPLE          0x0D      // humidity sample	   		   
#define I2C_HUMIDITY_MEAN60          0x0F      // humidity mean 60 sec  
#define I2C_HUMIDITY_MEAN            0x11      // humidity mean	   
#define I2C_HUMIDITY_MAX             0x13      // humidity max	   
#define I2C_HUMIDITY_MIN             0x15      // humidity min	   
#define I2C_HUMIDITY_SIGMA           0x17      // humidity sigma        

#define I2C_TH_MAP_WRITABLE          0x1F
#define I2C_TH_ONESHOT               0x1F      // sample mode (bool)
#define I2C_TH_ADDRESS               0x20      // i2c bus address (short unsigned int)
#define I2C_TH_TEMPERATUREADDRESS    0x21      // i2c bus address (short unsigned int)
#define I2C_TH_HUMIDITYADDRESS       0x22      // i2c bus address (short unsigned int)


///////////////////////////////////////////////////////////////////////////////////////////////////
// End register definition 
///////////////////////////////////////////////////////////////////////////////////////////////////
