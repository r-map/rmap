/**********************************************************************
Copyright (C) 2017  Marco Baldinetti <m.baldinetti@digiteco.it>
authors:
Paolo Paruno <p.patruno@iperbole.bologna.it>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of 
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/

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
