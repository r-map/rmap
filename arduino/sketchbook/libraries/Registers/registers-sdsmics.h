///////////////////////////////////////////////////////////////////////////////////////////////////
// I2C SDS011 registers
///////////////////////////////////////////////////////////////////////////////////////////////////

#define I2C_SDSMICS_DEFAULTADDRESS        36                      //7 bit address

// all bit to 1 => 0xFFFF or 65535 for int 
#define MISSINTVALUE 0xFFFF

// offset to write signed int in unsigned int
#define OFFSET 32768


//
#define I2C_SDSMICS_COMMAND               0xFF
#define I2C_SDSMICS_COMMAND_ONESHOT_START    1
#define I2C_SDSMICS_COMMAND_ONESHOT_STOP     2
#define I2C_SDSMICS_COMMAND_STOP             3
#define I2C_SDSMICS_COMMAND_SAVE             4

#define I2C_SDSMICS_VERSION               0x00      // Version

#define I2C_SDS011_PM25                   0x01      // pm 2.5
#define I2C_SDS011_PM10                   0x03      // pm 10
#define I2C_SDS011_MINPM25                0x05      // pm 2.5 mean 
#define I2C_SDS011_MINPM10                0x07      // pm 10  mean
#define I2C_SDS011_MEANPM25               0x09      // pm 2.5 min
#define I2C_SDS011_MEANPM10               0x0B      // pm 10  min
#define I2C_SDS011_MAXPM25                0x0D      // pm 2.5 max
#define I2C_SDS011_MAXPM10                0x0F      // pm 10  max
#define I2C_SDS011_SIGMAPM25              0x11      // pm 2.5 sigma
#define I2C_SDS011_SIGMAPM10              0x13      // pm 10  sigma

#define I2C_MICS4514_CO                   0x15      // CO
#define I2C_MICS4514_NO2                  0x17      // NO2
#define I2C_MICS4514_MINCO                0x19      // CO  mean 
#define I2C_MICS4514_MINNO2               0x1B      // NO2 mean
#define I2C_MICS4514_MEANCO               0x1D      // CO  min
#define I2C_MICS4514_MEANNO2              0x1F      // NO2 min
#define I2C_MICS4514_MAXCO                0x21      // CO  max
#define I2C_MICS4514_MAXNO2               0x23      // NO2 max
#define I2C_MICS4514_SIGMACO              0x25      // CO  sigma
#define I2C_MICS4514_SIGMANO2             0x27      // NO2 sigma
#define I2C_MICS4514_CORESISTANCE         0x29      // CO mics resistance (for calibration)
#define I2C_MICS4514_NO2RESISTANCE        0x2B      // NO2 mics resistance (for calibration)
#define I2C_MICS4514_PM25SAMPLE            0x2D     // PM2.5 sds011 sample (for calibration)
#define I2C_MICS4514_PM10SAMPLE           0x2F      // PM10 sds011 sample (for calibration)

#define I2C_SDSMICS_MAP_WRITABLE          0x40
#define I2C_SDSMICS_ONESHOT               0x40      // saple mode (bool)
#define I2C_SDSMICS_ADDRESS               0x41      // i2c bus address (short unsigned int)

#define NO2NUMPOINTS                      0X42      // num of points for NO2 calibration (MAX 5) (short unsigned int)
#define NO2CONCENTRATIONS                 0x43      // NO2 calibration concentrations (MAX 5) (float)
#define NO2RESISTENCES                    0x6B      // NO2 calibration resistances (MAX 5) (float)

#define CONUMPOINTS                       0x7F      // num of points for CO calibration (MAX 5) (short unsigned int)
#define COCONCENTRATIONS                  0x80      // CO calibration concentrations (MAX 5) (float)
#define CORESISTENCES                     0x94      // CO calibration resistances (MAX 5) (float)

#define PM25NUMPOINTS                     0xA8      // num of points for PM 2.5 calibration (MAX 5) (short unsigned int)
#define PM25CONCENTRATIONS                0xA9      // PM 2.5 calibration concentrations (MAX 5) (float)
#define PM25SAMPLES                       0xBD      // PM 2.5 calibration resistances (MAX 5) (float)

#define PM10NUMPOINTS                     0xD1      // num of points for PM10 calibration (MAX 5) (short unsigned int)
#define PM10CONCENTRATIONS                0xD2      // PM10 calibration concentrations (MAX 5) (float)
#define PM10SAMPLES                       0xE5      // PM10 calibration resistances (MAX 5) (float)


///////////////////////////////////////////////////////////////////////////////////////////////////
// End register definition 
///////////////////////////////////////////////////////////////////////////////////////////////////
