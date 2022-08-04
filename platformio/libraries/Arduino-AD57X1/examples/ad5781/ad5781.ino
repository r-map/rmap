#include "src/Arduino-AD57X1/src/ad57X1.h"

#define CS_AD5781              7

AD5781 ad5781(CS_AD5781, &SPI);
// AD5781 pidDac(CS_AD5781, &SPI1);   // Use SPI1 on MCUs, that have multiple SPI controllers (e.g. Teensy)
// AD5781 pidDac(CS_AD5781, &SPI, 30*1000*1000);    // Use an SPI clock frequency of 30 MHz, the maximum specified for the DAC.
                                                    // If you experience trasmit errors, this might be due to your board layout.

void setup() {
  ad5781.begin();   // Set the pin modes
  SPI.begin();
  ad5781.setOffsetBinaryEncoding(true);   // Set the input encoding to offset binary. Default is 2s complement (false).

  //ad5781.setReferenceInputRange(true);   // Enable only if the reference voltage span is greater than 10 V, Default: false.

  //ad5781.setOutputClamp(true);    // Enable to clamp the output to GND using a 6k resistor
  //ad5781.updateControlRegister()  // Call this function after changing the controll register, to update the settings

  //ad5781.setTristateMode(true);   // Disconnect the output (output will be tri-state)
  //ad5781.updateControlRegister()  // Call this function after changing the controll register, to update the settings

  //ad5781.setInternalAmplifier(true) // Enable the internal amplifier. This setup allows connecting an external amplifier in a gain of 2 configuration. See the datasheet for details.

  ad5781.enableOutput();    // Turn on the DAC. After startup the output will be clamped to GND and disconnected (tri-state mode)
                            // No need to call updateControlRegister(), because this is a convenience function, which does all that for you.

  ad5781.setValue(0x3FFFF); // Set to full scale output
}

void loop() {

}
