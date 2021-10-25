void setup() {
   Serial.begin(115200);
   Serial1.begin(115200);
}

void loop() {
   if (Serial1.available()) {
      Serial.write(Serial1.read());
   }

   if (Serial.available()) {
      char f = Serial.peek();
      if (f == 'X') {
         pinMode(5, OUTPUT);
         digitalWrite(5, LOW);
         delay(100);
         digitalWrite(5, HIGH);
         delay(1200);
         digitalWrite(5, LOW);
      }
   }
   if (Serial.available()) {
      Serial1.write(Serial.read());
   }
}
