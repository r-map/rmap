/**
 * sps-30 Library Header file
 *
 * Copyright (c) January 2019, Paul van Haastrecht
 *
 * All rights reserved.
 * Will work with either UART or I2c communication.
 * The I2C link has a number of restrictions. See detailed document
 *
 * Development environment specifics:
 * Arduino IDE 1.9
 *
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 **********************************************************************
 * Version 1.1 / August 2019
 * Simplified for RMAP and no blocking use
 *
 * Version 1.0   / January 2019
 * - Initial version by paulvha
 *
 * Version 1.2   / January 2019
 * - added force serial1 when TX = RX = 8
 * - added flag  INCLUDE_SOFTWARE_SERIAL to exclude software Serial
 *
 * version 1.2.1 / February 2019
 * - added flag in sps30.h SOFTI2C_ESP32 to use SoftWire on ESP32 in case of SCD30 and SPS30 working on I2C
 *
 * version 1.3.0 / February 2019
 * - added check on the I2C receive buffer. If at least 64 bytes it try to read ALL information else only MASS results
 * - added || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__) for small footprint
 *
 * version 1.3.1 / April 2019
 * - corrected bool stop() {return(Instruct(SER_STOP_MEASUREMENT));}
 *
 * version 1.3.2 / May 2019
 * - added support to detect SAMD I2C buffer size
 *********************************************************************
*/
#ifndef SPS30_H
#define SPS30_H

#include "config.h"

#include "Arduino.h"                // Needed for Stream

#ifndef USEARDUINOLOGLIB
#define SPS30LOGV(x, ...)
#define SPS30LOGD(x, ...)
#else
#include <ArduinoLog.h>
#define SPS30LOGV(x, ...) ({Log.verbose(x, ##__VA_ARGS__);})
#define SPS30LOGD(x, ...) ({Log.verbose(x, ##__VA_ARGS__);})
#endif

#if defined INCLUDE_I2C

    #if defined SOFTI2C_ESP32       // in case of SCD30
        #include <SoftWire/SoftWire.h>
    #else
        #include "Wire.h"           // for I2c
    #endif

    /* Version 1.3.0
     *
     * The read results is depending on the Wire / I2c buffer size, defined in Wire.h.
     *
     * The buffer size needed for each float value is 6 (LSB + MSB + CRC ++++ LSB + MSB + CRC)
     * To read all values an I2C buffer of atleast 6 x 10 = 60 bytes is needed
     * On many boards the default buffer size is set to 32 in Wire.h, thus providing 5 valid float values.
     * You can increase (if memory size allows) that yourself in Wire.h
     *
     * Here we determine the buffersize and the calculation is done in the constructor for sps30
     * IF the buffer size is less than 64 only the MASS values are provided. This is for I2C only!!
     *
     * From a sketch you can check the impact by calling I2C_expect(), which will return the number of valid float values.
     */
    #define I2C_LENGTH 32

    #if defined BUFFER_LENGTH           // Arduino  & ESP8266 & Softwire
        #undef  I2C_LENGTH
        #define I2C_LENGTH  BUFFER_LENGTH
    #endif

    #if defined I2C_BUFFER_LENGTH       // ESP32
        #undef  I2C_LENGTH
        #define I2C_LENGTH  I2C_BUFFER_LENGTH
    #endif

    /* version 1.3.2 added support for SAMD SERCOM detection */
    #if defined ARDUINO_ARCH_SAMD || defined ARDUINO_ARCH_SAM21D         // Depending on definition in wire.h (RingBufferN<256> rxBuffer;)
        #undef  I2C_LENGTH
        #define I2C_LENGTH  256
    #endif

#endif // INCLUDE_I2C

//#if defined INCLUDE_UART
//
//    #if defined INCLUDE_SOFTWARE_SERIAL
//
//        /* version 1.3.2 added support for SAMD SERCOM detection */
//      #if not defined ARDUINO_ARCH_SAMD  && not defined ARDUINO_ARCH_SAM21D      // NO softserial on SAMD
//         #include <SoftwareSerial.h>        // softserial
//      #endif // not defined ARDUINO_ARCH_SAMD
//
//    #endif // INCLUDE_SOFTWARE_SERIAL
//
//#endif // INCLUDE_UART

/**
 *  The communication it can be :
 *   I2C_COMMS              use I2C communication
 *   SERIALPORT             use UART communication
 *   NONE                   No port defined
 *
 * Softserial has been left in as an option, but as the SPS30 is only
 * working on 115K the connection will probably NOT work on any device.
 */
typedef enum serial_port {
			  NONE = 0,
			  I2C_COMMS = 1,
			  SERIALPORT = 2
};

/* structure to return all values */
typedef struct sps_values
{
    float   MassPM1;        // Mass Concentration PM1.0 [μg/m3]
    float   MassPM2;        // Mass Concentration PM2.5 [μg/m3]
    float   MassPM4;        // Mass Concentration PM4.0 [μg/m3]
    float   MassPM10;       // Mass Concentration PM10 [μg/m3]
    float   NumPM0;         // Number Concentration PM0.5 [#/cm3]
    float   NumPM1;         // Number Concentration PM1.0 [#/cm3]
    float   NumPM2;         // Number Concentration PM2.5 [#/cm3]
    float   NumPM4;         // Number Concentration PM4.0 [#/cm3]
    float   NumPM10;        // Number Concentration PM4.0 [#/cm3]
    float   PartSize;       // Typical Particle Size [μm]

  /*
   *  An answer on the typical size from Sensirion:
   *
   *  The typical particle size (TPS) is not a function of the other SPS30 outputs,
   *  but an independent output. It gives an indication on the average particle diameter
   *  in the sample aerosol. Such output correlates with the weighted average of the number
   *  concentration bins measured with a TSI 3330 optical particle sizer.
   *  Following this definition, lighter aerosols will have smaller TPS values than heavier aerosols.
   *  The reactiveness of this output increases with the particle statistics: a larger number of
   *  particles in the environment will generate more rapidly meaningful
   *  TPS values than a smaller number of particles (i.e., clean air).
   *
   */
};

/* needed for conversion float IEE754 */
typedef union {
    byte array[4];
    float value;
} ByteToFloat;

/* needed for auto interval timing */
typedef union {
    byte array[4];
    uint32_t value;
} ByteToU32;

/*************************************************************/
/* error codes */
#define ERR_OK          0x00
#define ERR_DATALENGTH  0X01
#define ERR_UNKNOWNCMD  0x02
#define ERR_ACCESSRIGHT 0x03
#define ERR_PARAMETER   0x04
#define ERR_OUTOFRANGE  0x28
#define ERR_CMDSTATE    0x43
#define ERR_TIMEOUT     0x50
#define ERR_PROTOCOL    0x51


#if not defined SMALLFOOTPRINT

typedef struct Description {
    uint8_t code;
    char    desc[80];
};

/* error descripton */
struct Description ERR_desc[10] =
{
  {ERR_OK, "All good"},
  {ERR_DATALENGTH, "Wrong data length for this command (too much or little data)"},
  {ERR_UNKNOWNCMD, "Unknown command"},
  {ERR_ACCESSRIGHT, "No access right for command"},
  {ERR_PARAMETER, "Illegal command parameter or parameter out of allowed range"},
  {ERR_OUTOFRANGE, "Internal function argument out of range"},
  {ERR_CMDSTATE, "Command not allowed in current state"},
  {ERR_TIMEOUT, "No response received within timeout period"},
  {ERR_PROTOCOL, "Protocol error"},
  {0xff, "Unknown Error"}
};
#endif // SMALLFOOTPRINT


/* Receive buffer length. Expected is 40 bytes max
 * but you never know in the future.. */
#if defined SMALLFOOTPRINT
#define MAXRECVBUFLENGTH 50         // for light boards
#else
#define MAXRECVBUFLENGTH 128
#endif

/*************************************************************/
/* SERIAL COMMUNICATION INFORMATION */
#define SER_START_MEASUREMENT       0x00
#define SER_STOP_MEASUREMENT        0x01
#define SER_READ_MEASURED_VALUE     0x03
#define SER_START_FAN_CLEANING      0x56
#define SER_RESET                   0xD3

#define SER_READ_DEVICE_INFO        0xD0    // GENERIC device request
#define SER_READ_DEVICE_PRODUCT_NAME    0xF1
#define SER_READ_DEVICE_ARTICLE_CODE    0xF2
#define SER_READ_DEVICE_SERIAL_NUMBER   0xF3

#define SER_AUTO_CLEANING_INTERVAL  0x80    // Generic autoclean request
#define SER_READ_AUTO_CLEANING          0x81    // read autoclean
#define SER_WRITE_AUTO_CLEANING         0x82    // write autoclean

#define SHDLC_IND   0x7e                   // header & trailer
#define TIME_OUT    5000                   // timeout to prevent deadlock read
#define RX_DELAY_MS 200                    // wait between write and read

/*************************************************************/
/* I2C COMMUNICATION INFORMATION  */
#define I2C_START_MEASUREMENT       0x0010
#define I2C_STOP_MEASUREMENT        0x0104
#define I2C_READ_DATA_RDY_FLAG      0x0202
#define I2C_READ_MEASURED_VALUE     0x0300
#define I2C_AUTO_CLEANING_INTERVAL  0x8004
#define I2C_SET_AUTO_CLEANING_INTERVAL      0x8005
#define I2C_START_FAN_CLEANING      0x5607
#define I2C_READ_ARTICLE_CODE       0xD025
#define I2C_READ_SERIAL_NUMBER      0xD033
#define I2C_RESET                   0xD304

#define SPS30_ADDRESS 0x69                 // I2c address
/***************************************************************/

class SPS30
{
  public:

#if defined INCLUDE_UART
    SPS30(Stream *serial=NULL);
#else
    SPS30(void);
#endif

    /**
     * @brief Initialize the communication port
     *
     * @param port : communication channel to be used (see sps30.h)
     */
    bool begin(serial_port port = I2C_COMMS); //If user doesn't specify I2C will be used

    /**
     * @brief : Perform SPS-30 instructions
     */
    bool probe();
    bool reset() {return(Instruct(SER_RESET));}
    bool start() {return(Instruct(SER_START_MEASUREMENT));}
    bool stop() {return(Instruct(SER_STOP_MEASUREMENT));}
    bool clean() {return(Instruct(SER_START_FAN_CLEANING));}

    /**
     * @brief : Set or get Auto Clean interval
     */
    uint8_t GetAutoCleanInt(uint32_t *val);
    uint8_t SetAutoCleanInt(uint32_t val);

    /**
     * @brief : retrieve Error message details
     */
    void GetErrDescription(uint8_t code, char *buf, int len);

    /**
     * @brief : retrieve device information from the SPS-30
     */
    uint8_t GetSerialNumber(char *ser, uint8_t len) {return(Get_Device_info( SER_READ_DEVICE_SERIAL_NUMBER, ser, len));}
    uint8_t GetArticleCode(char *ser, uint8_t len)  {return(Get_Device_info( SER_READ_DEVICE_ARTICLE_CODE, ser, len));}
    uint8_t GetProductName(char *ser, uint8_t len)  {return(Get_Device_info(SER_READ_DEVICE_PRODUCT_NAME, ser, len));}

    /**
     * @brief : retrieve all measurement values from SPS-30
     */
    uint8_t GetValues(struct sps_values *v);

#if defined INCLUDE_I2C
    /**
     * @brief : Return the expected number of valid values read from device
     *
     * This is depending on the buffer defined in Wire.h
     *
     * Return
     *  4 = Valid Mass values only
     * 10 = All values are expected to be valid
     */
    uint8_t I2C_expect();
#endif

  private:

    /** shared variables */
    uint8_t _Receive_BUF[MAXRECVBUFLENGTH]; // buffers
    uint8_t _Send_BUF[10];
    uint8_t _Receive_BUF_Length;
    uint8_t _Send_BUF_Length;
    serial_port _Sensor_Comms;  // comunication channel to use
    bool _started;              // indicate the measurement has started

    /** shared supporting routines */
    uint8_t Get_Device_info(uint8_t type, char *ser, uint8_t len);
    bool Instruct(uint8_t type);
    float byte_to_float(int x);
    uint32_t byte_to_U32(int x);
    uint8_t Serial_RX = 0, Serial_TX = 0; // softserial or Serial1 on ESP32

#if defined INCLUDE_UART
    /** UART / serial related */
    // calls
    bool setSerialSpeed();
    uint8_t ReadFromSerial();
    uint8_t SerialToBuffer();
    uint8_t SendToSerial();
    bool SHDLC_fill_buffer(uint8_t command, uint32_t parameter = 0);
    uint8_t SHDLC_calc_CRC(uint8_t * buf, uint8_t first, uint8_t last);
    int ByteStuff(uint8_t b, int off);
    uint8_t ByteUnStuff(uint8_t b);

    // variables
    Stream *_serial;        // serial port to use
#endif // INCLUDE_UART


#if defined INCLUDE_I2C
    /** I2C communication */
    void I2C_init();
    void I2C_fill_buffer(uint16_t cmd, uint32_t interval = 0);
    uint8_t I2C_ReadToBuffer(uint8_t count, bool chk_zero);
    uint8_t I2C_SetPointer_Read(uint8_t cnt, bool chk_zero = false);
    uint8_t I2C_SetPointer();
    bool I2C_Check_data_ready();
    uint8_t I2C_calc_CRC(uint8_t data[2]);
    uint8_t I2C_Max_bytes;
#endif // INCLUDE_I2C

};
#endif /* SPS30_H */
