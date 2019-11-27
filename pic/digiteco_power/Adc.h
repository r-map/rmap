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

