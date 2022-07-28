#include "register.h"

void setup() {
  // Open serial communications
  Serial.begin(115200);
  delay(5000);
  registerSetup();
}

void loop(void) {

  // Set up the default value. It will be used to populate the register if it doesn't exist.
  uavcan_register_Value_1_0 val = {0};
  uavcan_register_Value_1_0_select_natural16_(&val);
  val.natural16.value.count       = 1;
  val.natural16.value.elements[0] = UINT16_MAX;  // This means "undefined", per Specification, which is the default.


  val.natural16.value.elements[0] = 123;
  registerWrite("pippo.pluto.ciao", &val);
  registerRead("pippo.pluto.ciao", &val);
  Serial.println(val.natural16.value.elements[0]);
  delay(5000);
}
