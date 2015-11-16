#include <Wire.h>


#define I2C_GPS_ADDRESS                      0x20       

#include "registers-gps.h"

//**************************************************************
// Second byte will be from 'address'+1
unsigned char GPS_1_byte_read(unsigned char address)
{  
  Wire.beginTransmission(I2C_GPS_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();

  delay(300);
  
  Wire.requestFrom(I2C_GPS_ADDRESS, 1);
  if (Wire.available()<1) {
    Serial.println("errore");
    return 0;
  }
  return Wire.read();

}

int GPS_2_byte_read(unsigned char address)
{  
  unsigned char msb, lsb;
  
  Wire.beginTransmission(I2C_GPS_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();

  delay(300);
  
  Wire.requestFrom(I2C_GPS_ADDRESS, 2);
  while(Wire.available()<2)    ;
  msb = Wire.read();
  lsb = Wire.read();
  
  return (int) lsb<<8 | msb;
}

void GPS_8_byte_read(unsigned char address,int32_t* lat,int32_t* lon)
{  

  Wire.beginTransmission(I2C_GPS_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();

  delay(300);
  
  Wire.requestFrom(I2C_GPS_ADDRESS, 8);
  if (Wire.available()<8){
    Serial.println("errore") ;
    *lat=0;
    *lon=0;
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


void setup()
{
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output
  
  Serial.println("Starting:");

}

void loop()
{
  int32_t lat;
  int32_t lon;
  
  Serial.print("version:"); Serial.println(GPS_1_byte_read(I2C_GPS_VERSION) , DEC);

  Serial.print("numsat:"); Serial.println(GPS_1_byte_read(I2C_GPS_STATUS_NUMSATS) , DEC);

  GPS_8_byte_read(I2C_GPS_LOCATION,&lat,&lon);

  Serial.print("lat:"); Serial.println(lat);
  Serial.print("lon:"); Serial.println(lon);
  
  delay(1000);
}

