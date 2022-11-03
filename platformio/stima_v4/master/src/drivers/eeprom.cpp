/**
******************************************************************************
* @file    eeprom.c
* @author  AL
* @brief   This file includes a standard driver for external EEPROM memory.
*
@verbatim

@endverbatim
******************************************************************************
* @attention
*
* <h2><center>&copy; Copyright (c) 2021 Argo engineering.
* All rights reserved.</center></h2>
*
******************************************************************************
*/

#include <Arduino.h>
#include <string.h>
#include "drivers/eeprom.h"
#include "drivers/bsp_at24c64.h"
#include "stm32l4xx_hal.h"

// using namespace cpp_freertos;

// EEprom::EEprom(BinarySemaphore *_wireLock) {
// 	wireLock = _wireLock;
// }

EEprom::EEprom() {
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

		// if (wireLock->Take(Ticks::MsToTicks(10))) {
			int ok = HAL_I2C_Mem_Write(&I2CDEV, EEPROM_SLVADDR, waddr, I2C_MEMADD_SIZE_16BIT, &buf[2], wlen, 100);
			result = (HAL_StatusTypeDef) (result | HAL_I2C_Mem_Write(&I2CDEV, EEPROM_SLVADDR, waddr, I2C_MEMADD_SIZE_16BIT, &buf[2], wlen, 100));
			// wireLock->Give();
			if (len >= EEPAGESIZE) {
				delay(10);
				// osDelayTask(10);
			}
		// }
		// else return HAL_ERROR;
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

	// if (wireLock->Take(Ticks::MsToTicks(10))) {
		result = HAL_I2C_Master_Transmit(&I2CDEV, EEPROM_SLVADDR, buf, 2, 100);
		result = (HAL_StatusTypeDef) (result | HAL_I2C_Master_Receive(&I2CDEV, EEPROM_SLVADDR, rbuf, len, 100));
		// wireLock->Give();
		return result;
	// }
	// else return HAL_ERROR;
}
// #if 0
// /* copia in ram i parametri memorizzati in eeprom */
// AT24C64_Read(0, (uint8_t *) &RamPar, sizeof(PARAM_STR));
// if (CalcBlockCRC((uint8_t *) &RamPar, sizeof(PARAM_STR)) != 0)	 {		// check CRC
// 	//		memcpy(&RamPar,&DefaultPar,sizeof(PARAM_STR));
// 	//		RamPar.CRCval = CalcBlockCRC((uint8_t *) &RamPar, sizeof(PARAM_STR)-2);
// 	//		AT24C64_Write(0, (uint8_t *) &RamPar, sizeof(PARAM_STR));		// salva parametri e variabili
// 	flag.CRCerror = 1;
// 	_DBG_( "CRC error");
// }
// #endif
