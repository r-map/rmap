/**@file uart_driver.h */

/*********************************************************************
Copyright (C) 2022  Marco Baldinetti <marco.baldinetti@digiteco.it>
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

#ifndef _UART_DRIVER_H
#define _UART_DRIVER_H

#define UART_DRIVER_BAUD_RATE_DEFAULT (115200)

#define UART2_NVIC_INT_PREMPT_PRIORITY 6

#include "config.h"
#include "stima_config.h"
#include <stdint.h>
#include "core/net.h"
#include "ppp/ppp_hdlc.h"

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif

//UART driver
extern const UartDriver uartDriver;

//External interrupt related functions
error_t uartInitConfig(uint32_t baud);
error_t uartInit(void);
error_t uartDeInit(void);
void uartEnableIrq(void);
void uartDisableIrq(void);
void uartStartTx(void);

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
