#ifndef _BOOT_AREA_H
#define _BOOT_AREA_H
#ifdef    __cplusplus
extern "C" {
#endif

#include <pconfig.h>
#include "GenericTypeDefs.h"
#include "Hardware.h"

#define I2C_100_KHZ

void OpenI2C1Slave(BYTE bAddr);
void WriteI2C1(BYTE dataOut);
BYTE ReadI2C1(void);
void StartI2C1(void);
void RestartI2C1(void);
void StopI2C1(void);
void AckI2C1(void);
void IdleI2C1(void);
#define NackI2C1()      SSP1CON2bits.ACKDT=1;SSP1CON2bits.ACKEN=1;
void delay_ms (WORD value);
void delay_us (WORD value);

#ifdef    __cplusplus
}
#endif

#endif
