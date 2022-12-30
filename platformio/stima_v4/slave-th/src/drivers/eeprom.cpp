/**
  ******************************************************************************
  * @file    Eeprom.cpp
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   eeprom AT24C64 cpp source file
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

EEprom::EEprom(TwoWire *wire, BinarySemaphore *wireLock, uint8_t i2c_address)
{
	_wire = wire;
	_wireLock = wireLock;
	_i2c_address = i2c_address;
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
	if (_wireLock->Take(Ticks::MsToTicks(EEPROM_SEMAPHORE_MAX_WAITING_TIME_MS)))
	{
		_wire->beginTransmission(_i2c_address);
		if (_wire->endTransmission() == 0)
		{
			_wire->beginTransmission(_i2c_address);
			_wire->write(address >> 8);
			_wire->write(address & 0xFF);
			_wire->write(value);
			_wire->endTransmission();
			// Look time for write min 1.4 mS (On Byte)
			delay(WR_TIME_MS);
		}
		else
		{
			status = false;
		}
		_wireLock->Give();
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
	uint16_t iByte = 0;	// index byte to send
	if (_wireLock->Take(Ticks::MsToTicks(EEPROM_SEMAPHORE_MAX_WAITING_TIME_MS)))
	{
		while((eot == false)&&(status == true)) {
			_wire->beginTransmission(_i2c_address);
			if (_wire->endTransmission() == 0)
			{
				_wire->beginTransmission(_i2c_address);
				_wire->write(address >> 8);
				_wire->write(address & 0xFF);
				while(iByte<length) {
					_wire->write(buffer[iByte++]);
					// Update address && perform page pass for next byte
					if((++address % EEPAGESIZE) == 0) {
						// Switch next page (End of page)
						// Other Write rollUp to the init Byte of page
						break;
					}
				}
				_wire->endTransmission();
				// Test other data to write or eot
				eot = iByte >= length;
				// Look time for write min 3.6 mS (On Page)
				delay(WR_TIME_MS);
			}
			else
			{
				status = false;
			}
		}
		_wireLock->Give();
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
	if (_wireLock->Take(Ticks::MsToTicks(EEPROM_SEMAPHORE_MAX_WAITING_TIME_MS)))
	{
		_wire->beginTransmission(_i2c_address);
		if (_wire->endTransmission() == 0)
		{
			_wire->beginTransmission(_i2c_address);
			_wire->write(address >> 8);
			_wire->write(address & 0xFF);
			if (_wire->endTransmission() == 0)
			_wire->requestFrom((uint8_t)_i2c_address, 1);
			*value = _wire->read();
			_wire->endTransmission();
		}
		else
		{
			status = false;
		}
		_wireLock->Give();
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
	bool eor = false;
	uint16_t iIdx = 0;
	if (_wireLock->Take(Ticks::MsToTicks(EEPROM_SEMAPHORE_MAX_WAITING_TIME_MS)))
	{
		// Loop to end of receive total byte
		// Block read divise for PAGESIZE maxreceive bytes
		while (eor==false)
		{
			_wire->beginTransmission(_i2c_address);
			if (_wire->endTransmission() == 0)
			{
				_wire->beginTransmission(_i2c_address);
				_wire->write(address >> 8);
				_wire->write(address & 0xFF);
				if (_wire->endTransmission() == 0)
				{
					uint8_t requestLen;
					if(length > EEPAGESIZE) {
						requestLen = EEPAGESIZE;
						length -= EEPAGESIZE;
					} else {
						requestLen = length;
						eor = true;
					}
					_wire->requestFrom((uint8_t)_i2c_address, requestLen);
					for (uint16_t i = 0; i < requestLen; i++)
					{
						address++;
						buffer[iIdx++] = _wire->read();
					}
					_wire->endTransmission();
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
		}
		_wireLock->Give();
	}
	else
	{
		status = false;
	}
	return status;
}