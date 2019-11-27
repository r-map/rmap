#ifndef _HARDWARE_H
#define _HARDWARE_H
#ifdef    __cplusplus
extern "C" {
#endif

#include "GenericTypeDefs.h"

#define lo(x)               (BYTE)(x)
#define hi(x)               (BYTE)(x>>8u)
#define OFF         0x00
#define ON          0x01
#define PWR_CHGON   0x00
#define PWR_CHGOFF  0x01
#define     T_PANN          PORTAbits.RA0
#define     I_PANN          PORTAbits.RA1
#define     V_BATT          PORTAbits.RA2
#define     I_BATT          PORTAbits.RA3
#define     V_ALIM          PORTAbits.RA5
#define     TRISA_STARTUP   0b10111111
#define     LATA_STARTUP    0b00000000

#define     TRISB_STARTUP   0b00111111
#define     LATB_STARTUP    0b00000000

#define     PWR_CHGBAT      LATCbits.LATC0
#define     TRISC_STARTUP   0b11011110
#define     LATC_STARTUP    0b00000000
#define     LED_RD1         PORTDbits.RD0
#define     LED_RD0         PORTDbits.RD1
#define     TRISD_STARTUP   0b11111100
#define     LATD_STARTUP    0b00000000

#define     TRISE_STARTUP   0b00000111
#define     LATE_STARTUP    0b00000000
#define SET_CPU_Deep_Mode()     OSCCONbits.IDLEN=0
#define WatchDog()              ClrWdt()
#define EnableWatchDog()        WDTCONbits.SWDTEN = 1
void setup_hw_config(void);
void setup_hw_interrupt(void);

#ifdef    __cplusplus
}
#endif

#endif
