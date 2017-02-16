#include "hyt271.h"

/*
 * Function: HYT271_getHT
 * ----------------------------
 *   Returns the humidty and temperature from 4 byte raw data
 *
 *   raw_data: 4 byte raw data readed from the sensor: msb humidity | lsb humidity | msb temperature | lsb temperature
 *   humidity: pointer to humidity variable
 *   temperature: pointer to temperature variable
 *
 *   returns: value of humidity and temperature
 */
unsigned char HYT271_getHT(unsigned long raw_data, float *humidity, float *temperature) {
	// extract 14 bit humidity right adjusted (bit 0-14)
    *humidity = 100.0 / 0x3FFF * (raw_data >> 16 & 0x3FFF);
	
	// extract 14 bit temperature left adjusted (bit 2-16)
    *temperature = 165.0 / 0x3FFF * (((unsigned int) raw_data) >> 2) - 40;
	
	return 1;
}