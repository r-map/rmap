#include <Wire.h>

#define AFTERTRANSMISSIONDELAY       10

#include "registers.h"

//**************************************************************
// Second byte will be from 'address'+1
unsigned char GPS_1_byte_read(unsigned char address)
{  
  Wire.beginTransmission(I2C_WIND_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();

  delay(AFTERTRANSMISSIONDELAY);

  Wire.requestFrom(I2C_WIND_ADDRESS, 1);
  if (Wire.available()<1) {
    Serial.println("errore");
    return 0xFF;
  }
  return Wire.read();

}

unsigned int GPS_2_byte_read(unsigned char address)
{  
  unsigned char msb, lsb;
  
  Wire.beginTransmission(I2C_WIND_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();

  delay(AFTERTRANSMISSIONDELAY);

  Wire.requestFrom(I2C_WIND_ADDRESS, 2);
  if (Wire.available()<2){
    Serial.println("errore");
    return MISSINTVALUE;
  }
  msb = Wire.read();
  lsb = Wire.read();
  
  return (int) lsb<<8 | msb;
}

void GPS_8_byte_read(unsigned char address,int32_t* lat,int32_t* lon)
{  

  Wire.beginTransmission(I2C_WIND_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();

  delay(AFTERTRANSMISSIONDELAY);

  Wire.requestFrom(I2C_WIND_ADDRESS, 8);
  if (Wire.available()<8){
    Serial.println("errore") ;
    *lat=0xFFFFFFFF;
    *lon=0xFFFFFFFF;
    return;
  }

  *lat = Wire.read();
  *lat = *lat | ((int32_t)Wire.read())<<8 ;
  *lat = *lat | ((int32_t)Wire.read())<<16 ;
  *lat = *lat | ((int32_t)Wire.read())<<24 ;

  *lon = Wire.read();
  *lon = *lon | ((int32_t)Wire.read())<<8 ;
  *lon = *lon | ((int32_t)Wire.read())<<16 ;
  *lon = *lon | ((int32_t)Wire.read())<<24 ;
  
}


void oneshot()
{
  bool oneshot=true;
  Serial.print("version:"); Serial.println(GPS_1_byte_read(I2C_WIND_VERSION) , DEC);
  Serial.println("set oneshot mode");
  Wire.beginTransmission(I2C_WIND_ADDRESS);
  Wire.write(I2C_WIND_ONESHOT);
  Wire.write(oneshot);
  Wire.endTransmission();

  delay(100);

  Serial.println("stop oneshot");
  Wire.beginTransmission(I2C_WIND_ADDRESS);
  Wire.write(I2C_WIND_COMMAND);
  Wire.write(I2C_WIND_COMMAND_ONESHOT_STOP);
  Wire.endTransmission();

  delay(3100);

  for (int i =0; i<3;i++){

    Serial.println("restart oneshot");
    Serial.print("DD:"); Serial.println(GPS_2_byte_read(I2C_WIND_DD) , DEC);
    Serial.print("FF:"); Serial.println(GPS_2_byte_read(I2C_WIND_FF) , DEC);
    
    Serial.print("U :"); Serial.println((int)(GPS_2_byte_read(I2C_WIND_U)-OFFSET) , DEC);
    Serial.print("V :"); Serial.println((int)(GPS_2_byte_read(I2C_WIND_V)-OFFSET) , DEC);
    delay(1000);
  }

  for (int i =0; i<3;i++){

    Serial.println("start oneshot");
    Wire.beginTransmission(I2C_WIND_ADDRESS);
    Wire.write(I2C_WIND_COMMAND);
    Wire.write(I2C_WIND_COMMAND_ONESHOT_START);
    Wire.endTransmission();
    delay(2500);

    Serial.println("oneshot data:");
    Serial.print("DD:"); Serial.println(GPS_2_byte_read(I2C_WIND_DD) , DEC);
    Serial.print("FF:"); Serial.println(GPS_2_byte_read(I2C_WIND_FF) , DEC);
    Serial.print("U :"); Serial.println((int)(GPS_2_byte_read(I2C_WIND_U)-OFFSET) , DEC);
    Serial.print("V :"); Serial.println((int)(GPS_2_byte_read(I2C_WIND_V)-OFFSET) , DEC);
    
    Serial.println("stop oneshot");
    Wire.beginTransmission(I2C_WIND_ADDRESS);
    Wire.write(I2C_WIND_COMMAND);
    Wire.write(I2C_WIND_COMMAND_ONESHOT_STOP);
    Wire.endTransmission();
    delay(100);
  }

}

void continuos()
{
  
  Serial.print("version:"); Serial.println(GPS_1_byte_read(I2C_WIND_VERSION) , DEC);

  Serial.print("DD:"); Serial.println(GPS_2_byte_read(I2C_WIND_DD) , DEC);
  Serial.print("FF:"); Serial.println(GPS_2_byte_read(I2C_WIND_FF) , DEC);

  Serial.print("U :"); Serial.println((int)(GPS_2_byte_read(I2C_WIND_U)-OFFSET) , DEC);
  Serial.print("V :"); Serial.println((int)(GPS_2_byte_read(I2C_WIND_V)-OFFSET) , DEC);

  Serial.print("MEANU :");
  unsigned int value=GPS_2_byte_read(I2C_WIND_MEANU);
  if (value == 0xFFFF){
    Serial.println("Missing");
  }else{
    Serial.println((int)(value-OFFSET) , DEC);
  }

  Serial.print("MEANV :");
  value=GPS_2_byte_read(I2C_WIND_MEANV);
  if (value == 0xFFFF){
    Serial.println("Missing");
  }else{
    Serial.println((int)(value-OFFSET) , DEC);
  }

  Serial.print("PEAKGUSTU :"); 
  value=GPS_2_byte_read(I2C_WIND_PEAKGUSTU);
  if (value == 0xFFFF){
    Serial.println("Missing");
  }else{
    Serial.println((int)(value-OFFSET) , DEC);
  }
  Serial.print("PEAKGUSTV :"); 
  value=GPS_2_byte_read(I2C_WIND_PEAKGUSTV);
  if (value == 0xFFFF){
    Serial.println("Missing");
  }else{
    Serial.println((int)(value-OFFSET) , DEC);
  }

  Serial.print("LONGGUSTU :"); 
  value=GPS_2_byte_read(I2C_WIND_LONGGUSTU);
  if (value == 0xFFFF){
    Serial.println("Missing");
  }else{
    Serial.println((int)(value-OFFSET) , DEC);
  }
  Serial.print("LONGGUSTV :"); 
  value=GPS_2_byte_read(I2C_WIND_LONGGUSTV);
  if (value == 0xFFFF){
    Serial.println("Missing");
  }else{
    Serial.println((int)(value-OFFSET) , DEC);
  }

  Serial.print("MEANFF :"); 
  value=GPS_2_byte_read(I2C_WIND_MEANFF);
  if (value == 0xFFFF){
    Serial.println("Missing");
  }else{
    Serial.println((int)(value-OFFSET) , DEC);
  }

  delay(1000);
}

void setup()
{
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output
  
  Serial.println("Starting:");

  // sample 100Hz output 
  tone(5, 100);
}

void loop()
{
  oneshot();
  //continuos();
}
