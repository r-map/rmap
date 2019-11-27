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

