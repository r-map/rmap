/**
* @file spiDriverArduino.cpp
* @brief SPI driver
*
* @section License
*
* Copyright (C) 2010-2015 Oryx Embedded SARL. All rights reserved.
*
* This file is part of CycloneTCP Open.
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
* along with this program; if not, write to the Free Software Foundation,
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
* @author Oryx Embedded SARL (www.oryx-embedded.com)
* @version 1.6.5
**/

#include "arduino_spi_driver.h"
// #define waitspi() while(!(SPSR&(1<<SPIF)))

/**
* @brief SPI driver
**/
const SpiDriver spiDriver = {
  spiInit,
  spiSetMode,
  spiSetBitrate,
  spiAssertCs,
  spiDeassertCs,
  spiTransfer
};

/**
* @brief SPI initialization
* @return Error code
**/
error_t spiInit(void) {
  SPI.setMISO(SPI1_MISO);
  SPI.setMOSI(SPI1_MOSI);
  SPI.setSCLK(SPI1_CLK);
  pinMode(ETHERNET_CS_PIN, OUTPUT);
  digitalWrite(ETHERNET_CS_PIN, HIGH);
  // SPI.setBitOrder(MSBFIRST);

  SPI.begin();
  // SPI.setClockDivider(SPI_CLOCK_DIV4);
  return NO_ERROR;
}

/**
* @brief Set SPI mode
* @param mode SPI mode (0, 1, 2 or 3)
**/
error_t spiSetMode(uint_t mode) {
  SPI.setDataMode(mode);
  return NO_ERROR;
}

/**
* @brief Set SPI bitrate
* @param bitrate Bitrate value
**/
error_t spiSetBitrate(uint_t bitrate) {
  //Not implemented
  return ERROR_NOT_IMPLEMENTED;
}

/**
* @brief Assert CS
**/
void spiAssertCs(void) {
  digitalWrite(ETHERNET_CS_PIN, LOW);
  // SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  // sleep(100);
}

/**
* @brief Deassert CS
**/
void spiDeassertCs(void) {
  // sleep(100);
  // SPI.endTransaction();
  digitalWrite(ETHERNET_CS_PIN, HIGH);
  // sleep(100);
}

/**
* @brief Transfer a single byte
* @param[in] data The data to be written
* @return The data received from the slave device
**/
uint8_t spiTransfer(uint8_t data) {
  uint8_t recv = SPI.transfer(data);
  // Serial.print("send ");
  // Serial.println(data);
  // Serial.print("recv ");
  // Serial.println(recv);
  return recv;
}
