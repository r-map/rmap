/**@file config.h */

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

#ifndef _CONFIG_H
#define _CONFIG_H

#include "sensors_config.h"
#include "stima_config.h"

/*********************************************************************
* MODULE
*********************************************************************/
/*!
\def MODULE_MAIN_VERSION
\brief Module main version.
*/
#define MODULE_MAIN_VERSION   (4)

/*!
\def MODULE_MINOR_VERSION
\brief Module minor version.
*/
#define MODULE_MINOR_VERSION  (0)

/*!
\def MODULE_TYPE
\brief Type of module. It is defined in registers.h.
*/
#define MODULE_TYPE (STIMA_MODULE_TYPE_MASTER_GSM)

#define CONFIGURATION_EEPROM_ADDRESS (0)

#define SERIAL_DEBUG_BAUD_RATE   (115200)

#define ENABLE_I2C1              (true)
#define ENABLE_I2C2              (true)
#define ENABLE_QSPI              (false)
#define _HW_SETUP_GPIO_PRIVATE

#define PPP0_INTERFACE_NAME      ("ppp0")
#define ETH0_INTERFACE_NAME      ("eth0")

#if (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_GSM)
#define INTERFACE_0_NAME         PPP0_INTERFACE_NAME
#define INTERFACE_0_INDEX        (0)
#define PPP0_TIMEOUT_MS          (10000)
#define PPP0_BAUD_RATE_DEFAULT   (115200)
#define PPP0_BAUD_RATE_MAX       (3686400)

#elif (MODULE_TYPE == STIMA_MODULE_TYPE_MASTER_ETH)
#define INTERFACE_0_NAME         ETH0_INTERFACE
#define INTERFACE_0_INDEX        (0)
#endif

#define SYSTEM_REQUEST_QUEUE_LENGTH       (1)
#define SYSTEM_RESPONSE_QUEUE_LENGTH      (1)
#define SYSTEM_STATUS_QUEUE_LENGTH        (1)

#endif
