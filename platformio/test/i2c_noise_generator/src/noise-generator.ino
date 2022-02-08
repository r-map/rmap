#include <Arduino.h>

#define NOISE_DELAY_MS      (100)
#define ERROR_MIN_DELAY_MS  (100)
#define ERROR_MAX_DELAY_MS  (200)

#define LED_PIN             (3)
#define VCC_PIN             (4)
#define SCL_PIN             (SCL)
#define SDA_PIN             (SDA)

void setup () {
   Serial.begin(115200);
  pinMode(VCC_PIN, INPUT); // VCC
  pinMode(SCL_PIN, INPUT); // SCL
  pinMode(SDA_PIN, INPUT); // SDA
  pinMode(LED_PIN, OUTPUT);       // LED
  randomSeed(analogRead(0));
}

void loop () {
  // LED ON
  digitalWrite(LED_PIN, HIGH);
  Serial.println("ON");

  // SCL
  pinMode(SCL_PIN, OUTPUT);
  digitalWrite(SCL_PIN, LOW);
  delay(NOISE_DELAY_MS);
  pinMode(SCL_PIN, INPUT);

  // LED OFF
  digitalWrite(LED_PIN, LOW);
  Serial.println("OFF");

  delay(random(ERROR_MIN_DELAY_MS, ERROR_MAX_DELAY_MS));

  // LED ON
  digitalWrite(LED_PIN, HIGH);
  Serial.println("ON");

  // SDA
  pinMode(SDA_PIN, OUTPUT);
  digitalWrite(SDA_PIN, LOW);
  delay(NOISE_DELAY_MS);
  pinMode(SDA_PIN, INPUT);

  // LED OFF
  digitalWrite(LED_PIN, LOW);
  Serial.println("OFF");

  delay(random(ERROR_MIN_DELAY_MS, ERROR_MAX_DELAY_MS));
}
