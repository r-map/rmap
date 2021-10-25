#include <bq769x0.h>    // Library for Texas Instruments bq76920 battery management IC

#define BMS_ALERT_PIN 2     // attached to interrupt INT0
#define BMS_BOOT_PIN 7      // connected to TS1 input
#define BMS_I2C_ADDRESS 0x08
#define CC_CHANGED_PIN 13      // connected to TS1 input

bq769x0 BMS(bq76920, BMS_I2C_ADDRESS);    // battery management system object

float num_coulomb=0;
boolean CHARGE_ON=false;
const float MIN_TENS_CHARGE = 11000;


void setup()
{
  pinMode(BMS_ALERT_PIN, INPUT);
  pinMode(CC_CHANGED_PIN, OUTPUT);
  digitalWrite(CC_CHANGED_PIN,LOW);
  int err = BMS.begin(BMS_ALERT_PIN,BMS_BOOT_PIN);
  Serial.begin(9600);
  BMS.setTemperatureLimits(-20, 45, 0, 45);
  BMS.setShuntResistorValue(5);
  BMS.setShortCircuitProtection(14000, 200);  // delay in us
  BMS.setOvercurrentChargeProtection(8000, 200);  // delay in ms
  BMS.setOvercurrentDischargeProtection(8000, 320); // delay in ms
  BMS.setCellUndervoltageProtection(2600, 2); // delay in s
  BMS.setCellOvervoltageProtection(3750, 2);  // delay in s

  BMS.setBalancingThresholds(0, 3300, 20);  // minIdleTime_min, minCellV_mV, maxVoltageDiff_mV
  BMS.setIdleCurrentThreshold(100); //era 100
  BMS.enableAutoBalancing();
  BMS.setBatteryCapacity(2200);
  //BMS.update();  // should be called at least every 250 ms
  
}

void loop()
{
  int dato;
  byte registro;
  BMS.update();  // should be called at least every 250 ms  
  
  //BMS.printRegisters();  
  Serial.print("Vbat: ");
  Serial.print(BMS.getCellVoltage(1));
  Serial.print(" - ");
  Serial.print(BMS.getCellVoltage(2));
  Serial.print(" - ");
  Serial.print(BMS.getCellVoltage(5));
  Serial.print(" = ");
  Serial.print(BMS.getBatteryVoltage());
  Serial.print("mV - ");
  Serial.print("Temp: ");
  Serial.println(BMS.getTemperatureDegC(1)); 
  Serial.print("CHARGE ");  
  Serial.println(CHARGE_ON);  
  
  if (CHARGE_ON==false)
  {
    if (BMS.getBatteryVoltage() < MIN_TENS_CHARGE) CHARGE_ON=true;
  }
  else
  {
    if (BMS.enableCharging()==false)
    {
      CHARGE_ON=false;
      BMS.enableDischarging(); 
      delay(500);
    }
  }
  

  
  //Lettura Coulomb se è disponibile un nuovo dato
  /*
  if (BMS.readRegisterBit(0x00,7)==1)
  {
    int Val = (BMS.readRegister(0x32) << 8) | BMS.readRegister(0x33);
    float Current = Val * (8.44 / 5.0);  // mA (5.0 di default)
    Serial.print("Coulomb: "); 
    Serial.print(Val); 
    Serial.print(" / Corrente: "); 
    Serial.println(Current);     
    BMS.writeRegisterBit(0x00, 7, 0);
    delay(10);  
  }  
  */
  Serial.print("Coulomb: ");
  Serial.print(BMS.getSOC());  
  Serial.print(" / Corrente: ");
  Serial.println(BMS.getBatteryCurrent());
  
  delay(100);
}
