/**
  ******************************************************************************
  * @file    Eeprom.cpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   eeprom AT24C64 cpp sorce file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (C) 2022  Moreno Gasperini <m.gasperini@digiteco.it>
  * All rights reserved.</center></h2>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the GNU General Public License
  * as published by the Free Software Foundation; either version 2
  * of the License, or (at your option) any later version.
  * 
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  * 
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
  * <http://www.gnu.org/licenses/>.
  * 
  ******************************************************************************
*/


#include "drivers/eeprom.h"

EEprom::EEprom()
{
}

EEprom::EEprom(TwoWire *_wire, BinarySemaphore *_wireLock, uint8_t _i2c_address)
{
	wire = _wire;
	wireLock = _wireLock;
	i2c_address = _i2c_address;
}

/**
 * @brief 		Write a number of data byte into EEPROM
 * @param[in]	address, EEPROM data address
 * @param[in]	value, data value to write
 * @return		true if success, otherwise false returned.
 */
bool EEprom::Write(uint16_t address, uint8_t value)
{
	bool status = true;
	if (wireLock->Take(Ticks::MsToTicks(1000)))
	{
		wire->beginTransmission(i2c_address);
		if (wire->endTransmission() == 0)
		{
			wire->beginTransmission(i2c_address);
			wire->write(address >> 8);
			wire->write(address & 0xFF);
			wire->write(value);
			wire->endTransmission();
		}
		else
		{
			status = false;
		}
		wireLock->Give();
	}
	else
	{
		status = false;
	}
	return status;
}

/**
 * @brief 		Write a number of data byte into EEPROM
 * @param[in]	address, EEPROM data address
 * @param[in]	buffer, source data buffer location
 * @param[in]	length, buffer length
 * @return		true if success, otherwise false returned.
 */
bool EEprom::Write(uint16_t address, uint8_t *buffer, uint16_t length)
{
	bool status = true;	// status ok
	bool eot = false;	// end of transmission
	u_int8_t iByte = 0;	// index byte to send
	if (wireLock->Take(Ticks::MsToTicks(1000)))
	{
		while((eot == false)&&(status == true)) {
			bool eop = false;	// end of page
			wire->beginTransmission(i2c_address);
			if (wire->endTransmission() == 0)
			{
				wire->beginTransmission(i2c_address);
				wire->write(address >> 8);
				wire->write(address & 0xFF);
				while(iByte<length) {
					wire->write(buffer[iByte++]);
					// Update address && perform page pass for next byte
					if((++address % EEPAGESIZE) == 0) {
						// Switch next page
						eop = true;
						break;
					}
				}
				wire->endTransmission();
				// Test other data to write or eot
				eot = iByte >= length;
				// Look time page switch (eop need also with eot)
				if(eop) delay(10);
			}
			else
			{
				status = false;
			}
		}
		wireLock->Give();
	}
	else
	{
		status = false;
	}
	return status;
}

/**
 * @brief 		Read a single byte from EEPROM
 * @param[in]	address, EEPROM data address
 * @param[in]	value, destination data buffer location
 * @return		true if success, otherwise false returned.
 */
bool EEprom::Read(uint16_t address, uint8_t *value)
{
	bool status = true;
	if (wireLock->Take(Ticks::MsToTicks(1000)))
	{
		wire->beginTransmission(i2c_address);
		if (wire->endTransmission() == 0)
		{
			wire->beginTransmission(i2c_address);
			wire->write(address >> 8);
			wire->write(address & 0xFF);
			if (wire->endTransmission() == 0)
			*value = wire->read();
			wire->endTransmission();
		}
		else
		{
			status = false;
		}
		wireLock->Give();
	}
	else
	{
		status = false;
	}
	return status;
}

/**
 * @brief 		Read a number of data byte from EEPROM
 * @param[in]	address, EEPROM data address
 * @param[in]	read_buffer, destination data buffer location
 * @param[in]	length, buffer length
 * @return		true if success, otherwise false returned.
 */
bool EEprom::Read(uint16_t address, uint8_t *buffer, uint16_t length)
{
	bool status = true;
	if (wireLock->Take(Ticks::MsToTicks(1000)))
	{
		wire->beginTransmission(i2c_address);
		if (wire->endTransmission() == 0)
		{
			wire->beginTransmission(i2c_address);
			wire->write(address >> 8);
			wire->write(address & 0xFF);
			if (wire->endTransmission() == 0)
			{
				wire->requestFrom((uint8_t)i2c_address, length);
				for (uint8_t i = 0; i < length; ++i)
				{
					buffer[i] = wire->read();
				}
				wire->endTransmission();
			}
			else
			{
				status = false;
			}
		}
		else
		{
			status = false;
		}
		wireLock->Give();
	}
	else
	{
		status = false;
	}
	return status;
}