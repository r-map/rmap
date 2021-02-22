HardwareSerial Serial1(PA10, PA9);

void setup() {
  Serial.begin();
  Serial1.begin(115200);
}

void loop() {
  Serial.println("USB Ciao!");
  delay(500);
  Serial1.println("UART Ciao!");
  delay(500);
}
