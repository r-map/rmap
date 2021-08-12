//HardwareSerial Serial(PA10, PA9);
HardwareSerial Serial1(PB11, PB10);

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);
}

void loop() {
  Serial.println("serial Ciao!");
  delay(500);
  Serial1.println("serial1 Ciao!");
  delay(500);
}
