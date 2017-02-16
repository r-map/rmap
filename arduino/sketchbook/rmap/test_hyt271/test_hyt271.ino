#include <Wire.h>
#include <hyt271.h>

void test_read_ht() {
  float humidity;
  float temperature;

  // Request 4 bytes at default address 0x28: 2 bytes for Humidity and 2 bytes for Temperature
  Wire.requestFrom(I2C_HYT271_DEFAULT_ADDRESS, I2C_HYT271_READ_HT_DATA_LENGTH);

  if (Wire.available() == I2C_HYT271_READ_HT_DATA_LENGTH) {
    HYT271_getHT((unsigned long) Wire.read() << 24 | (unsigned long) Wire.read() << 16 | (unsigned long) Wire.read() << 8 | (unsigned long) Wire.read(), &humidity, &temperature);    
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print("% - Temperature: ");
    Serial.print(temperature);
    Serial.write(0xB0); // °
    Serial.println("C");
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(400000L); // 400 KHz
}

void loop() {
  test_read_ht();
  delay(5000);
}
