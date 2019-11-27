#ifndef RAMDEFINEEXTERN_H
#define	RAMDEFINEEXTERN_H
#ifdef	__cplusplus
extern "C" {
#endif
#include "GenericTypeDefs.h"
#include "Adc.h"
extern st_adcint adcint;
extern WORD wHWMainDelay;
extern WORD wLWMainDelay;
extern float fVal;
extern BYTE i2c_data;
extern BYTE *bPtr;
extern volatile BYTE int_ADC_INT;
extern volatile BYTE timer_RUN;
extern volatile BYTE timer_SEC;
extern volatile BYTE secTestValue;
extern BOOL bEnabOFF;
#ifdef	__cplusplus
}
#endif
#endif

