#include <bq769x0.h>    // Library for Texas Instruments bq76920 battery management IC

#define BMS_ALERT_PIN 2       //Pin collegato al ALN (Allerta pin)
#define BMS_BOOT_PIN 7        //Pin BOOT connesso a TS1
#define BMS_I2C_ADDRESS 0x18  //Indirizzo I2C dell'integrato

bq769x0 BMS(bq76920, BMS_I2C_ADDRESS);    //Dichiarazione gestore batteria

float num_coulomb=0;
boolean CHARGE_ON=false;                  //False: Ricarica non in corso
const float MIN_TENS_CHARGE = 11000;      //Tensione minima al di sotto della quale avviene la ricarica (somma delle celle)


void setup()
{
  Serial.begin(115200);
  Serial.println("Starting UPS");
  Wire.begin();        // join I2C bus
  pinMode(BMS_ALERT_PIN, INPUT);

  /*
  pinMode(BMS_BOOT_PIN, OUTPUT);
  digitalWrite(BMS_BOOT_PIN, HIGH);
  Serial.println("Boot");
  delay(5000);   // wait 5 ms for device to receive boot signal (datasheet: max. 2 ms)
  Serial.println("Booted");
  digitalWrite(BMS_BOOT_PIN, LOW);
  */

  BMS.begin(BMS_ALERT_PIN,BMS_BOOT_PIN);
  BMS.setTemperatureLimits(-20, 45, 0, 45);
  BMS.setShuntResistorValue(5);
  BMS.setShortCircuitProtection(14000, 200);  // delay in us
  BMS.setOvercurrentChargeProtection(8000, 200);  // delay in ms
  BMS.setOvercurrentDischargeProtection(8000, 320); // delay in ms
  BMS.setCellUndervoltageProtection(2600, 2); // delay in s
  BMS.setCellOvervoltageProtection(3750, 2);  // delay in s

  BMS.setBalancingThresholds(0, 3300, 20);  // minIdleTime_min, minCellV_mV, maxVoltageDiff_mV
  BMS.setIdleCurrentThreshold(100); 
  BMS.enableAutoBalancing();        //Attivazione bilanciamento batterie
  //BMS.enableDischarging();
  //BMS.setBatteryCapacity(2200);     //Capacità batteria. usato per il calcolo della corrente
  Serial.println("End Setup");  
}

void loop()
{
  BMS.update();  //Queta funzione dovrebbe essere chiamata ogni 250ms per avere le indicazioni corrette
  
  //BMS.printRegisters();               //Utilizzato per scrivere tutti i registri
  Serial.print("Vbat: ");               //Si visualizza la tensione di ogni cella
  Serial.print(BMS.getCellVoltage(1));
  Serial.print(" - ");
  Serial.print(BMS.getCellVoltage(2));
  Serial.print(" - ");
  Serial.print(BMS.getCellVoltage(5));
  Serial.print(" = ");
  Serial.print(BMS.getBatteryVoltage());
  Serial.print("mV - ");
  Serial.print("Temp: ");               //Si visualizza la temperatura
  Serial.println(BMS.getTemperatureDegC(1)); 
  Serial.print("CHARGE ");              //Si indica se è in corso la ricarica oppure no 
  Serial.println(CHARGE_ON);  
  
  //Se la ricarica non è in corso, se la tensione scende al di sotto di MIN_TENS_CHARGE si attiva la ricarica
  if (CHARGE_ON==false) 
  {
    if (BMS.getBatteryVoltage() < MIN_TENS_CHARGE) 
    {
      CHARGE_ON=true;
      BMS.enableCharging(); 
    }
  }
  //Se non è in corso la ricarica, si attiva il normale funzionamento
  else
  {
    if (BMS.enableCharging()==false)
    {
      CHARGE_ON=false;
      BMS.enableDischarging(); 
      delay(500);
    }
  }
  

  
  //Lettura Coulomb se è disponibile un nuovo dato, da uasre se se vuole leggere
  //in autonomia il registro senza passare per la libreria
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
  ////Serial.print("Coulomb: ");
  ////Serial.print(BMS.getSOC());  
  Serial.print(" / Corrente: ");
  Serial.println(BMS.getBatteryCurrent());  
  delay(250);
}
