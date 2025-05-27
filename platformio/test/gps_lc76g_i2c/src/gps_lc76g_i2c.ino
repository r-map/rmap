#include <Wire.h>

#define I2C_MASTER_FREQ_HZ 100000
#define DATABUFFERLEN 120

#define DEVICE_ADDRESS 0x50
#define DEVICE_ADDRESS_R 0x54

uint32_t dataLength = 0;

esp_err_t i2c_write(uint8_t device_addr, const uint8_t *data, size_t data_len) {

  Wire.beginTransmission(device_addr);
  Wire.write(data, data_len);
  return Wire.endTransmission();
}

esp_err_t i2c_read(uint8_t device_addr, uint8_t *data, size_t data_len) {
  int RxIdx = 0;
  size_t returned = Wire.requestFrom(device_addr, data_len);
  if (returned == data_len){
    while(Wire.available()) {
      data[RxIdx++] = Wire.read();  // Receive a byte & push it into the RxBuffer
    }
    return 0;	
  }else{
    return 1;
  }
}

void reset(){
  vTaskDelay(pdMS_TO_TICKS(15));
 
  uint8_t none;
  //i2c_write(DEVICE_ADDRESS, &none, sizeof(none));
  //i2c_write(DEVICE_ADDRESS_R, &none, sizeof(none));
  i2c_write(DEVICE_ADDRESS, &none, 0);
  i2c_write(DEVICE_ADDRESS_R, &none, 0);
  dataLength = 0;  
}

void stepOne() {
  uint8_t data[] = { 0x08, 0x00, 0x51, 0xAA, 0x04, 0x00, 0x00, 0x00 };

  // A
  if (i2c_write(DEVICE_ADDRESS, data, sizeof(data)) != ESP_OK) {
    Serial.println("Failed to write data step one A");
    reset();
    return;
  }
  vTaskDelay(pdMS_TO_TICKS(15));

  // B
  uint8_t readData[4] = { 0 };

  if (i2c_read(DEVICE_ADDRESS_R, readData, sizeof(readData)) != ESP_OK) {
    Serial.println("Failed to read data step one B");
    reset();
    return;
  }

  dataLength = (readData[0]) | (readData[1] << 8) | (readData[2] << 16) | (readData[3] << 24);
  //Serial.printf("Data length to read one: %u\n", dataLength);
}

void stepTwo() {

  if (dataLength == 0) {
    return;
  }

  // A
  uint8_t data[] = { 0x00, 0x20, 0x51, 0xAA };

  uint32_t dataBufferLen=DATABUFFERLEN;
  uint8_t dataBuffer[DATABUFFERLEN];
  if (dataBufferLen > dataLength) {
    dataBufferLen=dataLength;
    dataLength=0; // terminate read loop
  } else {
    dataLength-=dataBufferLen;
  }
  uint8_t readData[4];
  readData[0]=(dataBufferLen & 0xFF);
  readData[1]=(dataBufferLen >> 8 & 0xFF);
  readData[2]=(dataBufferLen >> 16 & 0xFF);
  readData[3]=(dataBufferLen >> 24 & 0xFF);

  //Serial.printf("Data length to read two: %u\n", dataBufferLen);
  
  uint8_t dataToSend[sizeof(data) + sizeof(readData)];
  memcpy(dataToSend, data, sizeof(data));
  memcpy(dataToSend + sizeof(data), readData, sizeof(readData));
  delay(15);

  if (i2c_write(DEVICE_ADDRESS, dataToSend, sizeof(dataToSend)) != ESP_OK) {
    Serial.println("Failed to write step two A");
    reset();
    return;
  }

  delay(15);

  // B
  if (i2c_read(DEVICE_ADDRESS_R, dataBuffer, dataBufferLen) != ESP_OK) {
    Serial.println("Failed to read data step two B");
    reset();
    return;
  }

  for (uint32_t i = 0; i < dataBufferLen; i++) {
    Serial.print((char)dataBuffer[i]);
  }
  
}

void lc76gReadData() {
  stepOne();
  while (dataLength != 0) {
    stepTwo();
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Started");
  Wire.begin();
  Wire.setClock(I2C_MASTER_FREQ_HZ);
  Wire.setTimeOut(50);
  delay(1000);  
}

void loop() {
  lc76gReadData();
  delay(1000);
}
