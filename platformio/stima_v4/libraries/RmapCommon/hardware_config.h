/**@file hardware_config.h */

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

#ifndef _HARDWARE_CONFIG_H
#define _HARDWARE_CONFIG_H

#ifdef STIMA_SLAVE
#define SPI1_MOSI           (PA7)   // D11 PA7
#define SPI1_MISO           (PA6)   // D12 PA6
#define SPI1_CLK            (PA5)   // D13 PA5

#define SPI2_MOSI           (PB15)
#define SPI2_MISO           (PB14)
#define SPI2_CLK            (PB13)
#define SPI2_BUS_CLOCK_MHZ  (18)

#define I2C1_SDA            (PB9)   // D14 SDA / A4 PC1
#define I2C1_SCL            (PB8)   // D15 SCL A5 PC0
#define I2C1_BUS_CLOCK_HZ   (1000000L)

#define BUTTON_PIN          (PC13)
#define LED2_PIN            (PA5)
#define SDCARD_CS           (PB6)   // D10

#endif

#ifdef STIMA_MASTER
#define SPI1_MOSI           (PA7)   // D11 PA7
#define SPI1_MISO           (PA6)   // D12 PA6
#define SPI1_CLK            (PA5)   // D13 PA5

#define LED1_PIN            (PC7)
#define LED2_PIN            (PB7)   // A4 - SDA
#define LED3_PIN            (PB14)

#define ENC28J60_INT_PIN    (PD15)  // D9
#define ENC28J60_CS_PIN     (PD14)  // D10

#define ETHERNET_INT_PIN    (ENC28J60_INT_PIN)
#define ETHERNET_CS_PIN     (ENC28J60_CS_PIN)
#endif

#define HARDWARE_I2C      (ENABLE)

#endif