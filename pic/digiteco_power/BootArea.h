/*********************************************************************
Copyright (C) 2017  Marco Baldinetti <m.baldinetti@digiteco.it>
authors:
Marco Baldinetti <m.baldinetti@digiteco.it>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/

#ifndef _BOOT_AREA_H
#define _BOOT_AREA_H
#ifdef    __cplusplus
extern "C" {
#endif

#include <pconfig.h>
#include "GenericTypeDefs.h"
#include "Hardware.h"

#define I2C_100_KHZ

void OpenI2C1Slave(BYTE bAddr);
void WriteI2C1(BYTE dataOut);
BYTE ReadI2C1(void);
void StartI2C1(void);
void RestartI2C1(void);
void StopI2C1(void);
void AckI2C1(void);
void IdleI2C1(void);
#define NackI2C1()      SSP1CON2bits.ACKDT=1;SSP1CON2bits.ACKEN=1;
void delay_ms (WORD value);
void delay_us (WORD value);

#ifdef    __cplusplus
}
#endif

#endif
