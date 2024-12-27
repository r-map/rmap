/**
 * ADDED version 1.4
 * New firmware levels have been slipped streamed into the SPS30
 * The datasheet from March 2020 shows added / updated functions on new
 * firmware level. E.g. sleep(), wakeup(), status register are new
 *
 * On serial connection the new functions are accepted and positive
 * acknowledged on lower level firmware, but execution does not seem
 * to happen or should be expected.
 *
 * On I2C reading Status register gives an error on lower level firmware.
 * Sleep and wakeup are accepted and positive acknowledged on lower level
 * firmware, but execution does not seem to happen or should be expected.
 *
 * Starting version 1.4 of this driver a firmware level check has been implemented
 * and in case a function is called that requires a higher level than
 * on the current SPS30, it will return an error.
 * By setting INCLUDE_FWCHECK to 0, this check can be disabled
 */
#define INCLUDE_FWCHECK 1


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
#define USEARDUINOLOGLIB
