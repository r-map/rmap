/**
  ******************************************************************************
  * @file    bsp_at24c64.h
  * @author  AL
  * @brief   This file contains all the description of the external AT24C64
	*						memory in CEMP board.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 Argo engineering.
  * All rights reserved.</center></h2>
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_AT24C64_H
#define __BSP_AT24C64_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAL_I2C_MODULE_ENABLED
#define EEPROM_SLVADDR		(0xA0)
extern I2C_HandleTypeDef hi2c2;
#define I2CDEV hi2c2
#define	EEPROMSIZE 8192
#define EEPAGESIZE 32
#define PAGEMASK (EEPROMSIZE-EEPAGESIZE)
#else
	#error "I2C device not defined!"
#endif

#ifdef __cplusplus
}
#endif

#endif /* __BSP_AT24C64_H */
