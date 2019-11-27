#pragma config FOSC     = HSMP
#pragma config PLLCFG   = OFF
#pragma config PRICLKEN = OFF
#pragma config FCMEN    = OFF
#pragma config IESO     = OFF
#pragma config BOREN    = OFF
#pragma config BORV     = 190
#pragma config WDTEN    = SWON
#pragma config WDTPS    = 128
#pragma config CCP2MX   = PORTB3
#pragma config PBADEN   = OFF
#pragma config CCP3MX   = PORTE0
#pragma config HFOFST   = OFF
#pragma config T3CMX    = PORTB5
#pragma config P2BMX    = PORTD2
#pragma config MCLRE    = EXTMCLR
#pragma config STVREN   = ON
#pragma config LVP      = OFF
#pragma config XINST    = OFF
#pragma config DEBUG    = OFF
#pragma config PWRTEN   = ON
#include    <p18cxxx.h>
#include    <delays.h>
#include    "Hardware.h"
#include    "Adc.h"
#include    "BootArea.h"
#include "RamDefineExtern.h"
void setup_hw_config(void)
{
   LATA=0;
   LATB=0;
   LATC=0;
   LATD=0;
   LATE=0;
   TRISA  = TRISA_STARTUP;
   TRISB  = TRISB_STARTUP;
   TRISC  = TRISC_STARTUP;
   TRISD  = TRISD_STARTUP;
   TRISE  = TRISE_STARTUP;
   PMD0 = 0b11111110;
   PMD1 = 0b10011111;
   PMD2 = 0b00001110;
   RCON = 0b10011111;
   ADCON2 = 0b10110111;
   LATA   = LATA_STARTUP;
   LATB   = LATB_STARTUP;
   LATC   = LATC_STARTUP;
   LATD   = LATD_STARTUP;
   LATE   = LATE_STARTUP;
   ANSELA = 0b00101111;
   ANSELB = 0x00;
   ANSELC = 0x00;
   ANSELD = 0x00;
   ANSELE = 0x00;
}

void setup_hw_interrupt(void)
{
   T1CON = 0b00100101;
   PIE1 = 0b01001001;
   IPR1 = 0b00001001;
   IPR2 = 0b00000000;
   IPR3 = 0;
   IPR4 = 0;
   IPR5 = 0;
   INTCON3 = 0b11000000;
   INTCON2 = 0b00000101;
   INTCON = 0b11000000;
}