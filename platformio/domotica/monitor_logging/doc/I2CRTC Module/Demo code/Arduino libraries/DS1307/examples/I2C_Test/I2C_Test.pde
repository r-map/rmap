/**
 * I2Ctest.pde -- Tiny RTC module for Arduino
 *
 * 2012, Waiman Zhao, http://waiman.taobao.com/
 *
 */
#include <OneWire.h>
#include "Wire.h"
#include <WProgram.h>
#include <DS1307.h>
#include <avr/io.h>
extern "C" { 
#include "utility/twi.h"  // from Wire library, so we can do bus scanning
}

OneWire  ds(2);  // on pin 2

byte start_address = 1;
byte end_address = 127;
byte Tdata[12];
int sensorValue = 0;        // value read from the pot
int rtc[7];
float TT=0.0;
int val;
int e1=1;
int e2=1;



//*****************************************
//
//      I2C Scaner code
//
//*****************************************
// Scan the I2C bus between addresses from_addr and to_addr.
// On each address, call the callback function with the address and result.
// If result==0, address was found, otherwise, address wasn't found
// (can use result to potentially get other status on the I2C bus, see twi.c)
// Assumes Wire.begin() has already been called
void scanI2CBus(byte from_addr, byte to_addr, 
                void(*callback)(byte address, byte result) ) 
{
  byte rc;
  byte data = 0; // not used, just an address to feed to twi_writeTo()
  for( byte addr = from_addr; addr <= to_addr; addr++ ) {
    rc = twi_writeTo(addr, &data, 0, 1);
    if(rc==0) callback( addr, rc );
  }
}


// Called when address is found in scanI2CBus()
// Feel free to change this as needed
// (like adding I2C comm code to figure out what kind of I2C device is there)
void scanFunc( byte addr, byte result ) {
	if(addr==104) Serial.println("DS1307 on connect!");
	else if(addr==80) Serial.println("24c32 on connect!");
	else{
      Serial.print("addr: ");
      Serial.print(addr,DEC);
      Serial.print("\t HEX: 0x");
      Serial.print(addr,HEX);
      Serial.println( (result==0) ? "\t found!":"   ");
    }
//  Serial.print( (addr%4) ? "\t":"\n");
}


//*****************************************
//
//      24Cxx I2C EEPROM code
//
//*****************************************
void i2c_eeprom_write_byte( int deviceaddress, unsigned int eeaddress, byte data ) {
    int rdata = data;
    Wire.beginTransmission(deviceaddress);
    Wire.send((int)(eeaddress >> 8)); // MSB
    Wire.send((int)(eeaddress & 0xFF)); // LSB
    Wire.send(rdata);
    Wire.endTransmission();
}

// WARNING: address is a page address, 6-bit end will wrap around
// also, data can be maximum of about 30 bytes, because the Wire library has a buffer of 32 bytes
void i2c_eeprom_write_page( int deviceaddress, unsigned int eeaddresspage, byte* data, byte length ) {
    Wire.beginTransmission(deviceaddress);
    Wire.send((int)(eeaddresspage >> 8)); // MSB
    Wire.send((int)(eeaddresspage & 0xFF)); // LSB
    byte c;
    for ( c = 0; c < length; c++)
      Wire.send(data[c]);
    Wire.endTransmission();
}

byte i2c_eeprom_read_byte( int deviceaddress, unsigned int eeaddress ) {
    byte rdata = 0xFF;
    Wire.beginTransmission(deviceaddress);
    Wire.send((int)(eeaddress >> 8)); // MSB
    Wire.send((int)(eeaddress & 0xFF)); // LSB
    Wire.endTransmission();
    Wire.requestFrom(deviceaddress,1);
    if (Wire.available()) rdata = Wire.receive();
    return rdata;
}

// maybe let's not read more than 30 or 32 bytes at a time!
void i2c_eeprom_read_buffer( int deviceaddress, unsigned int eeaddress, byte *buffer, int length ) {
    Wire.beginTransmission(deviceaddress);
    Wire.send((int)(eeaddress >> 8)); // MSB
    Wire.send((int)(eeaddress & 0xFF)); // LSB
    Wire.endTransmission();
    Wire.requestFrom(deviceaddress,length);
    int c = 0;
    for ( c = 0; c < length; c++ )
      if (Wire.available()) buffer[c] = Wire.receive();
}


//*****************************************
//
//      DS1302 RTC showtime code
//
//*****************************************
void showtime(void){
  byte i;
  Serial.print("Time=");
  RTC.get(rtc,true);  
  for(int i=0; i<7; i++)        {
  Serial.print(rtc[i]);
  Serial.print(" ");
  }
  Serial.println(" ");
}


//*****************************************
//
//      measure battery function
//
//*****************************************
void readBatVcc(void){
    sensorValue = analogRead(A1);
    TT = sensorValue*0.0047;
    Serial.print("Backup Vcc: ");
    Serial.print(TT);
    Serial.print("V");
}


//*****************************************
//
// prints "Press any key" and returns when key is pressed 
//
//*****************************************
void press_any_key()
{
  Serial.println("\r\nPress any key to continue to test the DS18B20...");
  while( Serial.available() == 0 ); //wait for input
  Serial.read();                    //empty input buffer    
  return;
}
    
    
//*****************************************
//
//      standard Arduino setup()
//
//*****************************************
void setup()
{
    DDRC|=_BV(2) |_BV(3);
    PORTC |=_BV(3);
    Wire.begin();
    pinMode(3, INPUT);
    pinMode(4, OUTPUT);
    digitalWrite(3, HIGH);
    digitalWrite(4, LOW);
    Serial.begin(115200);
    

    Serial.println("\n");
    Serial.println("\n");
    Serial.println("\n");
    Serial.println("***********  I2C Connect  ***********");
    // start the scan, will call "scanFunc()" on result from each address
    scanI2CBus( start_address, end_address, scanFunc );



    Serial.println("");
    Serial.println("");
    Serial.println("--------  EEPROM Test  ---------");
    char somedata[] = "this is data from the eeprom"; // data to write
    i2c_eeprom_write_page(0x50, 0, (byte *)somedata, sizeof(somedata)); // write to EEPROM 
    delay(100); //add a small delay
    Serial.println("Written Done");    
    delay(10);
    Serial.print("Read EERPOM:");
    byte b = i2c_eeprom_read_byte(0x50, 0); // access the first address from the memory
    char addr=0; //first address
    
    while (b!=0) 
    {
      Serial.print((char)b); //print content to serial port
      if (b!=somedata[addr]){
       e1=0;
       break;
       }      
      addr++; //increase address
      b = i2c_eeprom_read_byte(0x50, addr); //access an address from the memory
    }
    
    
    Serial.println("");
    Serial.println("");
    Serial.println("------  DS11307 RTC Test  ------");  
    showtime();
    if(rtc[6]!=2012){
      RTC.stop();
      RTC.set(DS1307_SEC,1);
      RTC.set(DS1307_MIN,0);
      RTC.set(DS1307_HR,0);
      RTC.set(DS1307_DOW,4);
      RTC.set(DS1307_DATE,12);
      RTC.set(DS1307_MTH,1);
      RTC.set(DS1307_YR,12);
      RTC.start();
      Serial.println("SetTime:");
      showtime();    
    }
    int oldtime=rtc[0];
  
  
    Serial.println("");
    Serial.println("");
    val = digitalRead(3);   // read the input pin
    if(val==HIGH){
      Serial.println("-----  Reserve Power Test  -----");
    
      Serial.println("  Close POWER!:");
      PORTC &=~_BV(3);
      byte time;
      for(time=0;time<3;time++){
         digitalWrite(13,HIGH);
         delay(500);
         digitalWrite(13,LOW);
         delay(500);     
         readBatVcc();
         Serial.println(""); 
      }
        PORTC |=_BV(3);
       Serial.println("  POWER On!");
       Serial.println("");
    }
    delay(200);
    showtime();
    if(rtc[0]<oldtime) e2= rtc[0]+60-oldtime;
    else e2= rtc[0]-oldtime;


    Serial.println("");
    Serial.println("");
    Serial.println("===============  Done   ===============");
    if (e1==0) Serial.println("\t EEPROM Read error");
    else Serial.println("\t EEPROM OK");

    if (TT<2.5) Serial.println("\t Battery is LOW");
    else{
      if (e2>3) Serial.println("\t Crystal is too fast");
      else  if (e2>=2) Serial.println("\t DS1307 OK");
      else if  (e2==0) Serial.println("\t Crystal failure");
      else Serial.println("\t Crystal is too slow");
    }
    Serial.print("\t ");
    readBatVcc();
    Serial.println("");
    Serial.println("");
    val = digitalRead(3);   // read the input pin
    if(val==HIGH)  press_any_key();
}



//*****************************************
//
//      standard Arduino loop()
//
//*****************************************
void loop() 
{
    byte i;
    byte present = 0;
    unsigned int Temper=0;
    
    readBatVcc();
    
    ds.reset();
    ds.write(0xCC,1);
    ds.write(0x44,1);         // start conversion, with parasite power on at the end
    digitalWrite(13,HIGH);
    delay(500);
    digitalWrite(13,LOW);
    delay(500);
    present = ds.reset();
    ds.write(0xCC,1);    
    ds.write(0xBE);         // Read Scratchpad
    for ( i = 0; i < 9; i++) {           // we need 9 bytes
      Tdata[i] = ds.read();
    }    
    Temper = (Tdata[1]<<8 | Tdata[0]);
    TT =Temper*0.0625;
    
    if(TT>200){
     Serial.println("\t DS18B20 Not installed!");
    }else{
      Serial.print("\t Temperature=");
      Serial.println(TT);
    }
    val = digitalRead(3);   // read the input pin
    if(val==LOW)   showtime(); 
    Serial.println("");
}
