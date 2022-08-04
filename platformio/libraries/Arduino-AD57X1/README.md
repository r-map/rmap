# Arduino-AD57X1
This library is still work in progress and not all features of the DACs are supported. Basic functionality is implemented. More documentation will be added soon.

#### Examples

This is a simple example, to get going. Check out the more in depth examples in [/examples/](./examples/)

```C++
#include "src/Arduino-AD57X1/src/ad57X1.h"

#define CS_AD5781              7
AD5781 ad5781(CS_AD5781, &SPI);   // Use AD5791 if using the 20 bit version

void setup() {
  ad5781.begin();   // Set the pin modes
  SPI.begin();
  ad5781.setOffsetBinaryEncoding(true);   // Set the input encoding to offset binary. Default is 2s complement (false).

  ad5781.enableOutput();    // Turn on the DAC. After startup the output will be clamped to GND and disconnected (tri-state mode)
  ad5781.setValue(0x3FFFF); // Set to full scale output
}

void loop() {

}
```
