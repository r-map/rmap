/**
 * @file main.h
 * @brief CycloneTCP configuration file
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2022 Oryx Embedded SARL. All rights reserved.
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
 * @version 2.1.4
 **/

#ifndef _MAIN_H
#define _MAIN_H

#include "task_config.h"
#include "hardware_config.h"

#include <STM32FreeRTOS.h>
#include "thread.hpp"

#include "core/net.h"
#include "drivers/spi/arduino_spi_driver.h"
#include "drivers/ext/arduino_interrupt_driver.h"
#include "drivers/eth/enc28j60_driver.h"
#include "dhcp/dhcp_client.h"
// #include "ipv6/slaac.h"
// #include "mqtt/mqtt_client.h"
// #include "http/http_client.h"
#include "tls.h"
#include "tls_cipher_suites.h"
#include "hardware/stm32l4xx/stm32l4xx_crypto.h"
#include "rng/trng.h"
#include "rng/yarrow.h"
#include "mydebug.h"

#include "tasks/led_task.h"
#include "tasks/hardware_task.h"
#include "tasks/ethernet_task.h"
#include "tasks/mqtt_task.h"

uint8_t seed[32];
YarrowContext yarrowContext;

error_t initCPRNG(void);

#endif
