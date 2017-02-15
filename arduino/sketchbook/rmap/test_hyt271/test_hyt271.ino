#include <Wire.h>
#include <hyt271.h>

void setup() {
  Serial.begin(115200);
  Wire.begin();
}

void loop() {
  double humidity;
  double temperature;

  Wire.beginTransmission(I2C_HYT271_DEFAULT_ADDRESS);
  Wire.requestFrom(I2C_HYT271_DEFAULT_ADDRESS, I2C_HYT271_READ_DATA_LENGTH);

  if (Wire.available() == I2C_HYT271_READ_DATA_LENGTH) {
    uint16_t raw_data;

    // humidity
    raw_data = Wire.read();
    raw_data = raw_data << 8 | Wire.read();
    raw_data = raw_data &= 0x3FFF;
    humidity = 100.0 / 0x3FFF * raw_data;

    // temperature
    raw_data = Wire.read();
    raw_data = raw_data << 6 | Wire.read() >> 2;
    temperature = 165.0 / 0x3FFF * raw_data - 40;
    
    Wire.endTransmission();
    
    Serial.print(humidity);
    Serial.print(" % - Temperature: ");
    Serial.print(temperature);
    Serial.print(" C\r\n");
  }
  else {
    Serial.println("Not enough bytes available on wire.");
  }

  delay(2000);
}
