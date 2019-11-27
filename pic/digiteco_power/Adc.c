#include <p18cxxx.h>
#include "Hardware.h"
#include "Adc.h"
#include "BootArea.h"
#include "RamDefineExtern.h"
void TASK_ADC_INTERNO(void)
{
   if(int_ADC_INT)
   {
      int_ADC_INT = 0;
      if(adcint.bNumConv<65)
      {
         if(adcint.bNumConv++) adcint.wValue += ADCINT_ReadValue();
      }
      else
      {            
         adcint.wData[ADCINT_GetMux()] = adcint.wValue;
         switch(ADCINT_GetMux())
         {
            case ADC_INT_CH_VPAN:
               adcint.fVPan = ((adcint.fVPan * 4) + (adcint.wValue / 2307.26)) / 5;
               if(adcint.fVPan<1.5)
               {
                  adcint.fVPan = 0.0;
                  if(bEnabOFF)
                  {
                     PWR_CHGBAT = PWR_CHGOFF;
                     secTestValue = 0;
                  }
               }
               else
               {
                  if(bEnabOFF)
                  {
                     if(secTestValue>=0xFB)
                        PWR_CHGBAT = PWR_CHGOFF;
                     else
                        PWR_CHGBAT = PWR_CHGON;
                  }
                  else
                     PWR_CHGBAT = PWR_CHGON;
               }
               break;
            case ADC_INT_CH_IPAN:
               if(adcint.fVPan<1.5)
                  adcint.fIPan = 0.0;
               else
               {
                  fVal = (adcint.wValue / 13107.2) * 2.5385;
                  adcint.fIPan = ((adcint.fIPan * 4) + fVal) / 5;
               }
               break;
            case ADC_INT_CH_VBAT:
               if(((PWR_CHGBAT==PWR_CHGOFF)&&((secTestValue==0xFF)||(secTestValue==0x00)))||
                       (adcint.fVPan<=1.5))
               {
                  adcint.fVBat = ((adcint.fVBat * 4) + (adcint.wValue / 4179.1)) / 5;
                  if(adcint.fVBat<1.5) adcint.fVBat = 0.0;
                  if(adcint.fVBat>13.1) adcint.fCBat = 100.0;
                  else if(adcint.fVBat>13.02) { adcint.fCBat = 98.73624 + ((adcint.fVBat - 13.02) * 18.05371429); }
                  else if(adcint.fVBat>12.92) { adcint.fCBat = 94.667243 + ((adcint.fVBat - 12.92) * 40.68997); }
                  else if(adcint.fVBat>12.81) { adcint.fCBat = 90.23132 + ((adcint.fVBat - 12.81) * 40.32657273); }
                  else if(adcint.fVBat>12.66) { adcint.fCBat = 83.93915757 + ((adcint.fVBat - 12.66) * 41.94774952); }
                  else if(adcint.fVBat>12.52) { adcint.fCBat = 77.13744076 + ((adcint.fVBat - 12.52) * 48.5836915); }
                  else if(adcint.fVBat>12.38) { adcint.fCBat = 64.27316294 + ((adcint.fVBat - 12.38) * 91.88769871); }
                  else if(adcint.fVBat>12.22) { adcint.fCBat = 49.35379644 + ((adcint.fVBat - 12.22) * 93.24604062); }
                  else if(adcint.fVBat>12.05) { adcint.fCBat = 39.47583948 + ((adcint.fVBat - 12.05) * 58.10562922); }
                  else if(adcint.fVBat>11.82) { adcint.fCBat = 29.40298507 + ((adcint.fVBat - 11.82) * 43.79501914); }
                  else if(adcint.fVBat>11.50) { adcint.fCBat = 19.62711864 + ((adcint.fVBat - 11.50) * 40.7327768); }
                  else if(adcint.fVBat>11.15) { adcint.fCBat = 14.54347826 + ((adcint.fVBat - 11.15) * 11.82241949); }
                  else if(adcint.fVBat>10.85) { adcint.fCBat = 9.713518356 + ((adcint.fVBat - 10.85) * 16.09986635); }
                  else if(adcint.fVBat>10.50) { adcint.fCBat = 4.838709677 + ((adcint.fVBat - 10.50) * 13.9280248); }
                  else adcint.fCBat = 0.0;
                  if(adcint.fCBat>100.0) adcint.fCBat = 100.0;
               }
               break;
            case ADC_INT_CH_IBAT:
               if(((PWR_CHGBAT==PWR_CHGOFF)&&((secTestValue==0xFF)||(secTestValue==0x00)))||
                       (adcint.fVPan<=1.5))
               {
                  fVal = (adcint.wValue / 13107.2);
                  if(fVal<1.0) fVal = 0.0;
                  else
                  {
                     fVal -= 2.5;
                     fVal *= 5405,405; // mV
                  }
                  adcint.fIBat = ((adcint.fIBat * 4) + fVal) / 5;
               }
               else
               {
                  if(PWR_CHGBAT==PWR_CHGON)
                  {
                     fVal = (adcint.wValue / 13107.2);
                     if(bEnabOFF)
                     {
                        if(fVal<2.505) bEnabOFF = FALSE;
                     }
                     else
                     {
                        if(fVal>2.505)
                        {
                           secTestValue = 0xFB;
                           bEnabOFF = TRUE;
                        }
                     }
                  }
               }
               break;
            case ADC_INT_CH_VOUT:
               adcint.fVOut = ((adcint.fVOut * 4) + (adcint.wValue / 6553.6)) / 5;
               break;
         }
         adcint.bNumConv = 0;
         adcint.wValue = 0;
         if((ADCINT_GetMux() + 1) >= ADC_INT_CH_USED)
            ADCINT_SetMux(0);
         else
            ADCINT_SetMux(ADCINT_GetMux() + 1);
      }
      startADCINTConv();
   }
}