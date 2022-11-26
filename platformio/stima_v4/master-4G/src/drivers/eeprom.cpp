/**@file eeprom.cpp */

/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <marco.baldinetti@alling.it>
authors:
Marco Baldinetti <marco.baldinetti@alling.it>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
<http://www.gnu.org/licenses/>.
**********************************************************************/

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
 * @param[in]	buffer, source data buffer location
 * @param[in]	length, buffer length
 * @return		true if success, otherwise false returned.
 */

bool EEprom::Write(uint16_t address, uint8_t *buffer, uint16_t length)
{
	bool status = false;

	if (wireLock->Take())
	{
		wire->beginTransmission(i2c_address);
		if (wire->endTransmission() == 0)
		{
			wire->beginTransmission(i2c_address);
			wire->write(address >> 8);
			wire->write(address & 0xFF);
			wire->write(buffer, length);
			if (wire->endTransmission() == 0)
			{
				status = true;
			}
		}
		wireLock->Give();
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
	bool status = false;
	
	if (wireLock->Take())
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
				if (wire->available() == length)
				{
					for (uint8_t i = 0; i < length; ++i)
					{
						buffer[i] = wire->read();
					}
					status = true;
				}
			}
		}
		wireLock->Give();
	}

	return status;
}