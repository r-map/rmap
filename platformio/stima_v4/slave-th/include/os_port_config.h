/**
 * @file os_port_config.h
 * @brief RTOS port configuration file
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2022 Oryx Embedded SARL. All rights reserved.
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

#ifndef _OS_PORT_CONFIG_H
#define _OS_PORT_CONFIG_H

// Accept GPL License
#define GPL_LICENSE_TERMS_ACCEPTED

// Select underlying RTOS
#define USE_FREERTOS

#undef USE_NO_RTOS
#undef USE_CHIBIOS
#undef USE_CMX_RTX
#undef USE_CMSIS_RTOS
#undef USE_CMSIS_RTOS2
#undef USE_SAFERTOS
#undef USE_THREADX
#undef USE_RTX
#undef USE_UCOS2
#undef USE_UCOS3
#undef USE_EMBOS
#undef USE_SYS_BIOS
#undef USE_ZEPHYR
#undef _WIN32
#undef __linux__
#undef __FreeBSD__

#endif
