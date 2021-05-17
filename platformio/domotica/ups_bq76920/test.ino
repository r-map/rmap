
#define BMS_ALERT_PIN D2       //Pin collegato al ALN (Allerta pin)
#define BMS_BOOT_PIN  A6       //Pin BOOT connesso a TS1

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting UPS");
  pinMode(BMS_BOOT_PIN, OUTPUT);
  digitalWrite(BMS_BOOT_PIN, HIGH);
  Serial.println("Boot");
  delay(5000);   // wait 5 ms for device to receive boot signal (datasheet: max. 2 ms)
  Serial.println("Booted");
  digitalWrite(BMS_BOOT_PIN, LOW);

  Serial.println("End Setup");  
}

void loop()
{
  delay(250);
}
