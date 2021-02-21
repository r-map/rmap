
#define MYPIN D4

void setup() {
  pinMode(MYPIN, OUTPUT); 
}

void loop() {
  digitalWrite(MYPIN, HIGH);
  delay(1000);
  digitalWrite(MYPIN, LOW);
  delay(1000);
  
}
