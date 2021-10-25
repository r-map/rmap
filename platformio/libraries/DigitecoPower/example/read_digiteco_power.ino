#include <debug_config.h>
#define SERIAL_TRACE_LEVEL SERIAL_TRACE_LEVEL_INFO

#include <debug.h>
#include <hardware_config.h>
#include <digiteco_power.h>

#define I2C_ADDRESS     (0x30)
#define I2C_RX_DATA     (4)

void setup() {
   SERIAL_BEGIN(115200);
   Wire.begin();
   Wire.setClock(I2C_BUS_CLOCK);
}

void loop() {
   float value;

   DigitecoPower::de_send(DIGITECO_POWER_DEFAULT_ADDRESS, DIGITECO_POWER_INPUT_VOLTAGE_ADDRESS);
   DigitecoPower::de_read(DIGITECO_POWER_DEFAULT_ADDRESS, &value);
   SERIAL_INFO(F("Input Voltage: %f V\r\n"),value);
   DigitecoPower::de_send(DIGITECO_POWER_DEFAULT_ADDRESS, DIGITECO_POWER_INPUT_CURRENT_ADDRESS);
   DigitecoPower::de_read(DIGITECO_POWER_DEFAULT_ADDRESS, &value);
   SERIAL_INFO(F("Input Current: %f mA\r\n"),value);
   DigitecoPower::de_send(DIGITECO_POWER_DEFAULT_ADDRESS, DIGITECO_POWER_BATTERY_VOLTAGE_ADDRESS);
   DigitecoPower::de_read(DIGITECO_POWER_DEFAULT_ADDRESS, &value);
   SERIAL_INFO(F("Battery Voltage: %f V\r\n"),value);
   DigitecoPower::de_send(DIGITECO_POWER_DEFAULT_ADDRESS, DIGITECO_POWER_BATTERY_CURRENT_ADDRESS);
   DigitecoPower::de_read(DIGITECO_POWER_DEFAULT_ADDRESS, &value);
   SERIAL_INFO(F("Battery Current: %f mA\r\n"),value);
   DigitecoPower::de_send(DIGITECO_POWER_DEFAULT_ADDRESS, DIGITECO_POWER_BATTERY_CHARGE_ADDRESS);
   DigitecoPower::de_read(DIGITECO_POWER_DEFAULT_ADDRESS, &value);
   SERIAL_INFO(F("Battery Charge: %f %%\r\n"),value);
   DigitecoPower::de_send(DIGITECO_POWER_DEFAULT_ADDRESS, DIGITECO_POWER_OUTPUT_VOLTAGE_ADDRESS);
   DigitecoPower::de_read(DIGITECO_POWER_DEFAULT_ADDRESS, &value);
   SERIAL_INFO(F("Output Voltage: %f V\r\n"),value);

   delay(1000);
}
