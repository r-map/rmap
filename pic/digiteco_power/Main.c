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
