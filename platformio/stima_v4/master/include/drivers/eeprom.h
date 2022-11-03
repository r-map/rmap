/**
  ******************************************************************************
  * @file    eeprom.h
  * @author  AL
  * @brief   This file contains the external eeprom memory functions driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 Argo engineering.
  * All rights reserved.</center></h2>
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __EEPROM_H
#define __EEPROM_H

// #include "os_port.h"
#include "stm32l4xx_hal.h"
// #include <STM32FreeRTOS.h>
// #include "ticks.hpp"
// #include "thread.hpp"
// #include "semaphore.hpp"

// using namespace cpp_freertos;

class EEprom {

public:
  // EEprom(BinarySemaphore *_wireLock);
  EEprom();
  /* AT24C64 functions */
  HAL_StatusTypeDef Write(uint16_t Addr, uint8_t *wbuf, uint16_t len);
  HAL_StatusTypeDef Read(uint16_t Addr, uint8_t *rbuf, uint16_t len);

protected:

private:
  // BinarySemaphore *wireLock;
};

#endif /* __EEPROM_H */
