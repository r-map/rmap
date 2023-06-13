#include "ad57X1.h"

#define CS_AD5791              D10

AD5791 ad5791(CS_AD5791, &SPI, 1*1000*1000,-1,0);
// AD5781 pidDac(CS_AD5781, &SPI1);   // Use SPI1 on MCUs, that have multiple SPI controllers (e.g. Teensy)
// AD5781 pidDac(CS_AD5781, &SPI, 30*1000*1000);    // Use an SPI clock frequency of 30 MHz, the maximum specified for the DAC.
                                                    // If you experience trasmit errors, this might be due to your board layout.

void setup() {

  Serial.begin(115200);

  Serial.println("Started");
  
  ad5791.begin(true);   // Set the pin modes
  //SPI.begin(); //enabled by true in previous begin
  //ad5791.setOffsetBinaryEncoding(true);   // Set the input encoding to offset binary. Default is 2s complement (false).

  //ad5791.setReferenceInputRange(true);   // Enable only if the reference voltage span is greater than 10 V, Default: false.

  //ad5791.setOutputClamp(true);    // Enable to clamp the output to GND using a 6k resistor
  //ad5791.updateControlRegister();  // Call this function after changing the controll register, to update the settings

  //ad5791.setTristateMode(true);   // Disconnect the output (output will be tri-state)
  //ad5791.updateControlRegister();  // Call this function after changing the controll register, to update the settings

  // without this the output range from 0V to 5V.
  ad5791.setInternalAmplifier(true); // Enable the internal amplifier. This setup allows connecting an external amplifier in a gain of 2 configuration setting output ranging fron -5V to 5V. See the datasheet for details.
  
  ad5791.enableOutput();    // Turn on the DAC. After startup the output will be clamped to GND and disconnected (tri-state mode)
                            // No need to call updateControlRegister(), because this is a convenience function, which does all that for you.

}

void loop() {

  Serial.println("zero");
  ad5791.setTension(0L);
  Serial.println(ad5791.readValue(),HEX); // re-read value
  delay(1000L*60*5);

  Serial.println("1V");
  ad5791.setTension(1000L);
  Serial.println(ad5791.readValue(),HEX); // re-read value
  delay(1000L*60*5);
 
  Serial.println("2V");
  ad5791.setTension(2000L);
  Serial.println(ad5791.readValue(),HEX); // re-read value
  delay(1000L*60*5);

  Serial.println("3V");
  ad5791.setTension(3000L);
  Serial.println(ad5791.readValue(),HEX); // re-read value
  delay(1000L*60*5);

}
