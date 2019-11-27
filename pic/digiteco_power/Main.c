#include "Compiler.h"
#include "Interrupt.h"
#include "BootArea.h"
#include "Adc.h"
#include "RamDefine.h"


void main (void) {
   setup_hw_config();
   EnableWatchDog();
   OpenI2C1Slave(0x30);
   setup_hw_interrupt();
   WatchDog();
   adcint.fVPan = 0.0;
   adcint.fIPan = 0.0;
   adcint.fVBat = 0.0;
   adcint.fIBat = 0.0;
   adcint.fCBat = 0.0;
   adcint.fVOut = 0.0;
   adcint.bNumConv=0;
   adcint.wValue=0;
   ADCINT_SetMux(ADC_INT_CH_VBAT);
   enableADCINT();
   startADCINTConv();
   SET_CPU_Deep_Mode();
   while(1) {
      WatchDog();
      TASK_ADC_INTERNO();
      RESET_INTERRUPT_FLAG();
   }
}