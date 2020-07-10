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

#ifndef _ADC_H
#define _ADC_H
#ifdef  __cplusplus
extern "C" {
#endif
#include "GenericTypeDefs.h"
#include "Hardware.h"
#define ADC_INT_CH_USED     5
#define ADC_INT_CH_VPAN     0
#define ADC_INT_CH_IPAN     1
#define ADC_INT_CH_VBAT     2
#define ADC_INT_CH_IBAT     3
#define ADC_INT_CH_VOUT     4
#define st_adcint struct e4s_adcint
st_adcint
{
    BYTE  bNumConv;
    WORD  wValue;
    WORD  wData[ADC_INT_CH_USED];
    float fVPan;
    float fIPan;
    float fVBat;
    float fIBat;
    float fCBat;
    float fVOut;
};

#define enableADCINT()      ADCON0bits.ADON = 1
#define startADCINTConv()   ADCON0bits.GO_NOT_DONE = 1
#define ADCINT_SetMux(X)    ADCON0bits.CHS = X
#define ADCINT_GetMux()     ADCON0bits.CHS
#define ADCINT_ReadValue()  (WORD)(((WORD)ADRESH<<8) | (WORD)ADRESL)

void  TASK_ADC_INTERNO(void);

#ifdef  __cplusplus
}
#endif

#endif
