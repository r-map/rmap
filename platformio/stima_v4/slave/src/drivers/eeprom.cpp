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
#include "drivers/bsp_at24c64.h"

EEprom::EEprom()
{
}

EEprom::EEprom(BinarySemaphore *_wireLock)
{
	wireLock = _wireLock;
}

/**
* @brief 		Write a number of data byte into EEPROM
* @param[in]	Addr, EEPROM data address
* @param[in]	wbuf, source data buffer location
* @param[in]	len, buffer length
* @return		HAL_OK if success, otherwise HAL_ERROR returned.
*/

HAL_StatusTypeDef EEprom::Write(uint16_t Addr, uint8_t *wbuf, uint16_t len) {
	uint8_t buf[EEPAGESIZE+sizeof(uint16_t)];
	HAL_StatusTypeDef result = HAL_OK;
	uint32_t wlen;
	uint16_t waddr = Addr;

	if ((Addr + len) > EEPROMSIZE) {		// address overflow
		return HAL_ERROR;
	}

	do {
		if ((waddr & PAGEMASK) != ((Addr + len -1) & PAGEMASK)) {		// write a cavallo di due pagine?
			wlen = ((waddr + EEPAGESIZE) & PAGEMASK) - waddr;
		}
		else {
			wlen = Addr + len - waddr;
		}

		if (wlen > EEPAGESIZE) {
			wlen = EEPAGESIZE;
		}

		buf[0] = (waddr >> 8) & 0xFF;
		buf[1] = waddr & 0xFF;
		memcpy(&buf[2], wbuf, wlen);
		wbuf += wlen;

		if (wireLock->Take(Ticks::MsToTicks(10))) {
			int ok = HAL_I2C_Mem_Write(&I2CDEV, EEPROM_SLVADDR, waddr, I2C_MEMADD_SIZE_16BIT, &buf[2], wlen, 100);
			result = (HAL_StatusTypeDef) (result | HAL_I2C_Mem_Write(&I2CDEV, EEPROM_SLVADDR, waddr, I2C_MEMADD_SIZE_16BIT, &buf[2], wlen, 100));
			wireLock->Give();
			if (len >= EEPAGESIZE) {
				osDelayTask(10);
			}
		}
		else return HAL_ERROR;
		waddr += wlen;
	} while (waddr != (Addr + len));
	return result;
}


/**
* @brief 		Read a number of data byte from EEPROM
* @param[in]	Addr, EEPROM data address
* @param[in]	rbuf, destination data buffer location
* @param[in]	len, buffer length
* @return		HAL_OK: if success, otherwise HAL_ERROR returned.
*/
HAL_StatusTypeDef EEprom::Read(uint16_t Addr, uint8_t *rbuf, uint16_t len) {
	HAL_StatusTypeDef result = HAL_OK;
	uint8_t buf[2];
	buf[0] = (Addr >> 8) & 0xFF;
	buf[1] = Addr & 0xFF;

	if (wireLock->Take(Ticks::MsToTicks(10))) {
		result = HAL_I2C_Master_Transmit(&I2CDEV, EEPROM_SLVADDR, buf, 2, 100);
		result = (HAL_StatusTypeDef) (result | HAL_I2C_Master_Receive(&I2CDEV, EEPROM_SLVADDR, rbuf, len, 100));
		wireLock->Give();
		return result;
	}
	else return HAL_ERROR;
}