/**@file arduino_spi_driver.h */

/*********************************************************************
<h2><center>&copy; Stimav4 is Copyright (C) 2023 ARPAE-SIMC urpsim@arpae.it</center></h2>
authors:
Marco Baldinetti <marco.baldinetti@digiteco.it>

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

#ifndef _CYCLONE_SPI_DRIVER_ARDUINO_H
#define _CYCLONE_SPI_DRIVER_ARDUINO_H

#include "config.h"
#include "stima_config.h"
#include <Arduino.h>
#include <SPI.h>
#include "core/net.h"
#include "error.h"
// #include <stdint.h>

//SPI driver
extern SPIClass SPI;
extern const SpiDriver spiDriver;

//SPI related functions
error_t spiInit(void);
error_t spiSetMode(uint_t mode);
error_t spiSetBitrate(uint_t bitrate);
void spiAssertCs(void);
void spiDeassertCs(void);
uint8_t spiTransfer(uint8_t data);

#endif
