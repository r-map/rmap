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

#include <p18cxxx.h>
#include "Interrupt.h"
#include "Hardware.h"
#include "RamDefineExtern.h"
#pragma code HIGH_INTERRUPT_VECTOR = 0x08
void high_ISR(void) {
   _asm goto interrupt_handler_high _endasm
}
#pragma code
#pragma code LOW_INTERRUPT_VECTOR = 0x18
void low_ISR(void) {
   _asm goto interrupt_handler_low _endasm
}
#pragma code
#pragma interrupt interrupt_handler_high
void interrupt_handler_high (void) {
   if (PIE1bits.SSPIE && PIR1bits.SSPIF) {
      i2c_data = SSPBUF;
      if (SSP1STATbits.R_NOT_W) {
         if ((SSP1STATbits.D_NOT_A) && (SSP1CON2bits.ACKSTAT)) {
         }
         else {
            SSPBUF = *bPtr;
            *bPtr++;
         }
      }
      else {
         if (!SSP1STATbits.D_NOT_A) {
         }
         else {
            switch(i2c_data)
            {
               case 0:
                  bPtr = (BYTE*) &adcint.fVPan;
                  break;
               case 1:
                  bPtr = (BYTE*) &adcint.fIPan;
                  break;
               case 2:
                  bPtr = (BYTE*) &adcint.fVBat;
                  break;
               case 3:
                  bPtr = (BYTE*) &adcint.fIBat;
                  break;
               case 4:
                  bPtr = (BYTE*) &adcint.fCBat;
                  break;
               case 5:
                  bPtr = (BYTE*) &adcint.fVOut;
                  break;
            }
         }
      }
      PIR1bits.SSPIF = 0;
      SSP1CON1bits.CKP = 1;
   }

   if(PIR1bits.TMR1IF)
   {
      TMR1H = 0xC8;
      TMR1L = 0x4D;
      PIR1bits.TMR1IF=0;
      PIE1bits.TMR1IE=1;

      timer_SEC++;
      if(timer_SEC>100)
      {
         timer_SEC=0;
         secTestValue++;
      }

      timer_RUN--;
      if(timer_RUN<6)
      {
         if((adcint.fVPan>20)&&(adcint.fCBat<33))
         {
            LED_RD0 = 1;
            LED_RD1 = 0;
            if(timer_RUN==1) timer_RUN=50;
         }
         else if((adcint.fVPan>20)&&(adcint.fCBat<66))
         {
            LED_RD0 = 1;
            LED_RD1 = 1;
            if(timer_RUN==1) timer_RUN=50;
         }
         else if((adcint.fVPan>20)&&(adcint.fCBat>66))
         {
            LED_RD0 = 0;
            LED_RD1 = 1;
            if(timer_RUN==1) timer_RUN=50;
         }
         else if((adcint.fVPan>14)&&(adcint.fCBat<33))
         {
            LED_RD0 = 1;
            LED_RD1 = 0;
            if(timer_RUN==1) timer_RUN=100;
         }
         else if((adcint.fVPan>14)&&(adcint.fCBat<66))
         {
            LED_RD0 = 1;
            LED_RD1 = 1;
            if(timer_RUN==1) timer_RUN=100;
         }
         else if((adcint.fVPan>14)&&(adcint.fCBat>66))
         {
            LED_RD0 = 0;
            LED_RD1 = 1;
            if(timer_RUN==1) timer_RUN=100;
         }
         else if((adcint.fVPan<14)&&(adcint.fCBat<33))
         {
            LED_RD0 = 1;
            LED_RD1 = 0;
            if(timer_RUN==1) timer_RUN=200;
         }
         else if((adcint.fVPan<14)&&(adcint.fCBat<66))
         {
            LED_RD0 = 1;
            LED_RD1 = 1;
            if(timer_RUN==1) timer_RUN=200;
         }
         else if((adcint.fVPan<14)&&(adcint.fCBat>66))
         {
            LED_RD0 = 0;
            LED_RD1 = 1;
            if(timer_RUN==1) timer_RUN=200;
         }
      }
      else
      {
         LED_RD0 = 0;
         LED_RD1 = 0;
      }
   }
}

#pragma interruptlow interrupt_handler_low
void interrupt_handler_low (void)
{
   if(PIR1bits.ADIF)
   {
      int_ADC_INT = 1;
      PIR1bits.ADIF = 0;
   }
}

void RESET_INTERRUPT_FLAG()
{
   INTCONbits.GIEL = 1;
   INTCONbits.GIEH = 1;

}
