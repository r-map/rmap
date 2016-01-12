///////////////////////////////////////////////////////////////////////////////////////////////////
// I2C GPS NAV registers
///////////////////////////////////////////////////////////////////////////////////////////////////
//
#define I2C_GPS_COMMAND               0xFF
#define I2C_GPS_COMMAND_TEST1            1
#define I2C_GPS_COMMAND_TEST2            2
#define I2C_GPS_COMMAND_TEST3            3

#define I2C_GPS_VERSION               0x00      // Version
#define I2C_GPS_STATUS_2DFIX          0x01      // 2dfix achieved
#define I2C_GPS_STATUS_3DFIX          0x02      // 3dfix achieved
#define I2C_GPS_STATUS_NUMSATS        0x03      // Number of sats in view

//GPS & navigation data
#define I2C_GPS_LOCATION              0x04   // current location 8 byte (lat, lon) int32_t
#define I2C_GPS_ALTITUDE              0x0C   // GPS altitude in meters (uint16_t)           (Read Only)
#define I2C_GPS_GROUND_SPEED          0x0E   // GPS ground speed in m/s*100 (uint16_t)      (Read Only)
#define I2C_GPS_GROUND_COURSE	      0x10   // GPS ground course (uint16_t)
#define I2C_GPS_TIME                  0x12   // UTC Time from GPS in hhmmss.sss * 100 (uint32_t)(unneccesary precision) (Read Only)

#define I2C_GPS_REG_DATETIME          0x16   // YEAR  (7 byte)
#define I2C_GPS_REG_YEAR              0x16   // YEAR  (uint16_t)
#define I2C_GPS_REG_MONTH             0x18   // MONTH (uint8_t)
#define I2C_GPS_REG_DAY               0x19   // DAY   (uint8_t)
#define I2C_GPS_REG_HOUR              0x1A   // HOUR  (uint8_t)
#define I2C_GPS_REG_MIN               0x1B   // MIN   (uint8_t)
#define I2C_GPS_REG_SEC               0x1C   // SEC   (uint8_t)

#define I2C_GPS_MAP_WRITABLE          0x1F


///////////////////////////////////////////////////////////////////////////////////////////////////
// End register definition 
///////////////////////////////////////////////////////////////////////////////////////////////////
