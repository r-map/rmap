#include <bq769x0.h>    // Library for Texas Instruments bq76920 battery management IC

#define BMS_ALERT_PIN 2     // attached to interrupt INT0
#define BMS_BOOT_PIN 7      // connected to TS1 input
#define BMS_I2C_ADDRESS 0x18

bq769x0 BMS(bq76920, BMS_I2C_ADDRESS);    // battery management system object

void setup()
{
  int err = BMS.begin(BMS_ALERT_PIN, BMS_BOOT_PIN);

  BMS.setTemperatureLimits(-20, 45, 0, 45);
  BMS.setShuntResistorValue(5);
  BMS.setShortCircuitProtection(14000, 200);  // delay in us
  BMS.setOvercurrentChargeProtection(8000, 200);  // delay in ms
  BMS.setOvercurrentDischargeProtection(8000, 320); // delay in ms
  BMS.setCellUndervoltageProtection(2600, 2); // delay in s
  BMS.setCellOvervoltageProtection(3650, 2);  // delay in s

  BMS.setBalancingThresholds(0, 3300, 20);  // minIdleTime_min, minCellV_mV, maxVoltageDiff_mV
  BMS.setIdleCurrentThreshold(100);
  BMS.enableAutoBalancing();
  BMS.enableDischarging();
}

void loop()
{
  BMS.update();  // should be called at least every 250 ms
  BMS.printRegisters();
}
