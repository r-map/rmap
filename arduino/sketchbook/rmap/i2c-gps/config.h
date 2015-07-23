//////////////////////////////////////////////////////////////////////////////
// i2C comm definitions
//
#define I2C_ADDRESS        0x20                      //7 bit address 0x40 write, 0x41 read

/* GPS Lead filter - predicts gps position based on the x/y speed. helps overcome the gps lag. */
#define GPS_LEAD_FILTER

/* Serial speed of the GPS */
#define GPS_SERIAL_SPEED 38400

/* GPS protocol 
 * NMEA			- Standard NMEA protocol GGA, GSA and RMC  sentences are needed
 * UBLOX		- U-Blox binary protocol, use the ublox config file (u-blox-config.ublox.txt) from the source tree 
 * MTK_BINARY16 - MTK binary protocol (DIYDrones v1.6) 
 * MTK_BINARY19 - MTK binary protocol (DIYDrones v1.9) 
 * MTK_INIT     - Initialize MTK GPS (if MTK_BINARY16 or 19 is not defined then it goes to NMEA, otherwise it goes for binary)
 */

//#define NMEA
#define UBLOX
//#define MTK_BINARY16
//#define MTK_BINARY19
//#define INIT_MTK_GPS

// activate debug on serial port
//#define DEBUGONSERIAL

#ifdef DEBUGONSERIAL
#define IF_SDEBUG(x) ({x;})
#else
#define IF_SDEBUG(x)
#endif

