#include <LoopbackStream.h>

LoopbackStream buffer;
int clickCount2 = 0;
int clickCount3 = 0;


void click2() {
  buffer.print("Button 2 was clicked ");
  buffer.print(++clickCount2);
  buffer.println(" times");
}

void click3() {
  buffer.print("Button 3 was clicked ");
  buffer.print(++clickCount3);
  buffer.println(" times");
}

void setup() {
  Serial.begin(9600);
  buffer.print("Hello, World");
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), click2, FALLING);
  attachInterrupt(digitalPinToInterrupt(3), click3, FALLING);
}

void loop() {
  if (buffer.available()) {
    Serial.write(buffer.read());
  }
  //delay(10);
}