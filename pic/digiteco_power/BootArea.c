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
