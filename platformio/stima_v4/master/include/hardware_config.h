/**
 * @file hardware_config.h
 * @brief Hardware's define
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2022 Marco Baldinetti. All rights reserved.
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
 * @author Marco Baldinetti <marco.baldinetti@alling.it>
 * @version 0.1
 **/

#ifndef _HARDWARE_CONFIG_H
#define _HARDWARE_CONFIG_H

#define LED1_PIN          (PC7)
#define LED2_PIN          (PB7)   // A4 - SDA
#define LED3_PIN          (PB14)

#define SPI1_MOSI         (PA7)   // D11 PA7
#define SPI1_MISO         (PA6)   // D12 PA6
#define SPI1_CLK          (PA5)   // D13 PA5

#define ENC28J60_INT_PIN  (PD15)  // D9
#define ENC28J60_CS_PIN   (PD14)  // D10

#define ETHERNET_INT_PIN  (ENC28J60_INT_PIN)
#define ETHERNET_CS_PIN   (ENC28J60_CS_PIN)

#endif
