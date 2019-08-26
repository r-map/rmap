/**
 * To EXCLUDE I2C communication, maybe for resource reasons,
 * comment out the line below.
 */
#define INCLUDE_I2C   1

/**
 * To EXCLUDE the serial communication, maybe for resource reasons
 * as you board does not have a seperate serial, comment out the line below
 * It will also exclude Software_serial
 */
//#define INCLUDE_UART 1

/**
 * On some IDE / boards software Serial is not available
 * comment out line below in that case
 *
 */
//#define INCLUDE_SOFTWARE_SERIAL 1

/**
 * If the platform is an ESP32 AND it is planned to connect an SCD30,
 * you have to remove the comments from the line below
 *
 * The standard I2C on an ESP32 does NOT support clock stretching
 * which is needed for the SCD30. You must have SCD30 library downloaded
 * from https://github.com/paulvha/scd30 and included in your sketch
 * (see examples)
 *
 * If you do not plan the SPS30 to run on I2C you can exclude the I2C in total
 */
//#define SOFTI2C_ESP32 1


/**
 *  Auto detect that some boards have low memory. (like Uno)
 */
//#if defined (__AVR_ATmega328__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)

#define SMALLFOOTPRINT 1

//
//    #if defined INCLUDE_UART
//        #undef INCLUDE_UART
//    #endif //INCLUDE_UART
//
//#endif // AVR definition check


// enable logging
//#define USEARDUINOLOGLIB
