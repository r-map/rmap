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

#ifndef _RAMDEFINE_H
#define _RAMDEFINE_H
#ifdef    __cplusplus
extern "C" {
#endif
#include "GenericTypeDefs.h"
st_adcint adcint;
WORD wHWMainDelay;
WORD wLWMainDelay;
float fVal;
BYTE i2c_data;
BYTE *bPtr;
volatile BYTE int_ADC_INT;
volatile BYTE timer_RUN=0;
volatile BYTE timer_SEC=0;
volatile BYTE secTestValue=0x00;
BOOL bEnabOFF = FALSE;

#ifdef    __cplusplus
}
#endif
#endif
