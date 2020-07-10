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

#include    <p18cxxx.h>
#include    "BootArea.h"
#include    "Hardware.h"
#include "RamDefineExtern.h"
void delay_ms (WORD value)
{
   for(wHWMainDelay=0; wHWMainDelay < value; wHWMainDelay++)
      delay_us(240);
}

void delay_us (WORD value)
{
   for(wLWMainDelay=0; wLWMainDelay < value; wLWMainDelay++)
      WatchDog();
}

void OpenI2C1Slave(BYTE bAddr)
{
   SSP1CON2 = 0b00000001;
   SSP1CON1 = 0b00110110;
   SSP1ADD = bAddr << 1;
   SSP1STAT = 0b10000000;
}

void WriteI2C1(BYTE dataOut)
{
   SSP1BUF = dataOut;
   while( SSP1STATbits.BF );
   IdleI2C1();
}

BYTE ReadI2C1(void)
{
   SSP1CON2bits.RCEN = 1;
   while (!SSP1STATbits.BF);
   return SSP1BUF;
}

void StartI2C1(void)
{
   SSP1CON2bits.SEN=1;
   while(SSP1CON2bits.SEN);
}

void RestartI2C1(void)
{
   SSP1CON2bits.RSEN=1;
   while(SSP1CON2bits.RSEN);
}

void StopI2C1(void)
{
   SSP1CON2bits.PEN=1;
   while(SSP1CON2bits.PEN);
}

void AckI2C1(void)
{
   SSP1CON2bits.ACKDT=0;
   SSP1CON2bits.ACKEN=1;
   while(SSP1CON2bits.ACKEN);
}

void IdleI2C1(void)
{
   while ((SSP1CON2 & 0x1F) | (SSP1STATbits.R_W));
}
