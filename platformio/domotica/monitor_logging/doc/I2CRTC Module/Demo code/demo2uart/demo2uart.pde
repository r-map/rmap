#include <OneWire.h>
#include <WProgram.h>
/* DS18S20 Temperature chip i/o */
#include <Wire.h>
#include <DS1307.h>

#include <util/delay.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#ifndef BAUD_RATE
#define BAUD_RATE   19200
#endif

#define	RX_SIZE		0x40      /* UART receive buffer size (must be 2^n ) <=256 */

#define	RX_MASK		(RX_SIZE-1)
#define	TX_MASK		(TX_SIZE-1)


OneWire  ds(3);  // on pin 3
byte Tdata[12];
int rtc[7];
int deviceaddress = 0x50;
int addr=0; //first address
int lastTime;
byte saveDate[5];
boolean full=true;
int ledPin = 13; 

uint8_t ReceivePtr;
uint8_t rx_buf[RX_SIZE];

int incomingByte = 0;	// for incoming serial data


void writeByte(unsigned int eeaddress, byte data ) {
    int rdata = data;
    Wire.beginTransmission(deviceaddress);
    Wire.send((byte)(eeaddress >> 8)); // MSB
    Wire.send((byte)(eeaddress & 0xFF)); // LSB
    Wire.send(rdata);
    Wire.endTransmission();
}

  // WARNING: address is a page address, 6-bit end will wrap around
  // also, data can be maximum of about 30 bytes, because the Wire library has a buffer of 32 bytes
void writePage(unsigned int eeaddresspage, byte* data, byte length ) {
    Wire.beginTransmission(deviceaddress);
    Wire.send((byte)(eeaddresspage >> 8)); // MSB
    Wire.send((byte)(eeaddresspage & 0xFF)); // LSB
    byte c;
    for ( c = 0; c < length; c++)
      Wire.send(data[c]);
    Wire.endTransmission();
}

byte readByte(unsigned int eeaddress ) {
    byte rdata = 0xFF;
    Wire.beginTransmission(deviceaddress);
    Wire.send((byte)(eeaddress >> 8)); // MSB
    Wire.send((byte)(eeaddress & 0xFF)); // LSB
    Wire.endTransmission();
    Wire.requestFrom(deviceaddress,1);
    if (Wire.available()) rdata = Wire.receive();
    return rdata;
}

  // maybe let's not read more than 30 or 32 bytes at a time!
void readBuffer(unsigned int eeaddress, byte *buffer, int length ) {
    Wire.beginTransmission(deviceaddress);
    Wire.send((byte)(eeaddress >> 8)); // MSB
    Wire.send((byte)(eeaddress & 0xFF)); // LSB
    Wire.endTransmission();
    Wire.requestFrom(deviceaddress,length);
    int c = 0;
    for ( c = 0; c < length; c++ )
      if (Wire.available()) buffer[c] = Wire.receive();
}


void clearRx(void){
    for(char i=0;i<RX_SIZE;i++)   rx_buf[i] = 0;
    ReceivePtr=0;
}


void showTime(void){
  byte i;
  RTC.get(rtc,true);
  for(int i=0; i<7; i++) {
      Serial.print(rtc[i]);
      Serial.print(" ");
   }
   Serial.println(" ");
}

void showRecord(void){
  full=true;
  unsigned int epaddr=0;
  byte i=0;
  byte length =sizeof(saveDate);
  unsigned int Temper=0;
  float TT=0.0;
   Serial.println("Show Record:");
  byte x=0;
  while(true){
    readBuffer(epaddr, (byte *)saveDate, length);
    Serial.print(x++,DEC);
    Serial.print(":\t");
    for(i=0;i<3;i++){
        if(saveDate[0] ==0xFF){
          Serial.println("\nThe End");
          return;
        }
        Serial.print(saveDate[i],DEC);
        Serial.print(" ");
    }
    Temper = (saveDate[4]<<8 | saveDate[3]);
    TT =Temper*0.0625;
    Serial.print("\t");
    Serial.println(TT);
    epaddr +=length;
  }  
}


void setDate(byte length,byte start){
   if(length==7){
    RTC.stop();
    RTC.set(DS1307_SEC,rx_buf[++start]);
    RTC.set(DS1307_MIN,rx_buf[++start]);
    RTC.set(DS1307_HR,rx_buf[++start]);
    RTC.set(DS1307_DOW,rx_buf[++start]);
    RTC.set(DS1307_DATE,rx_buf[++start]);
    RTC.set(DS1307_MTH,rx_buf[++start]);
    RTC.set(DS1307_YR,rx_buf[++start]);
    RTC.start();
  }
}


void readTemp(void){
    unsigned int Temper=0;
    float TT=0.0;
    byte i;
    ds.reset();
    ds.write(0xCC,1);    
    ds.write(0xBE);         // Read Scratchpad
    for ( i = 0; i < 9; i++) {           // we need 9 bytes
       Tdata[i] = ds.read();
    }
    Serial.print("Temperature=");
    Temper = (Tdata[1]<<8 | Tdata[0]);
    TT =Temper*0.0625;
    Serial.println(TT);
    //12	11	10	9
    //750       375     187.5   93.75
    //0.0625	0.125	0.25	0.5
}

void clearEProm(void){
  
  byte tmpbuffer[32]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                      0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                      0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                      0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
  unsigned int epaddr=0;
  while(epaddr<4065){
    digitalWrite(ledPin, LOW);  
    writePage(epaddr,tmpbuffer,32);
    epaddr +=32;
    delay(10);
    digitalWrite(ledPin, HIGH);  
    delay(10);
  }
   Serial.println("EEPRom Clear Done");
}
void setup() 
{
  Serial.begin(9600);
  DDRC|=_BV(2) |_BV(3);
  PORTC |=_BV(3);
  
  RTC.get(rtc,true);
  if(rtc[6]<2011){
    RTC.stop();
    RTC.set(DS1307_SEC,1);
    RTC.set(DS1307_MIN,52);
    RTC.set(DS1307_HR,16);
    RTC.set(DS1307_DOW,2);
    RTC.set(DS1307_DATE,25);
    RTC.set(DS1307_MTH,1);
    RTC.set(DS1307_YR,11);
    RTC.start();
  }
  // writePage(0, (byte *)somedata, sizeof(somedata)); // write to EEPROM 
    pinMode(ledPin,OUTPUT);
    ReceivePtr=0;
}

void loop() 
{
  byte i=0;
  byte present = 0;
  byte tmp =0;
  unsigned int Temper=0;
  float TT=0.0;
  ds.reset();
  ds.write(0xCC,1);
  ds.write(0x44,1);         // start conversion, with parasite power on at the end
  
//---------------------------------------------
  ReceivePtr=0;
	while (Serial.available() > 0) {
		// read the incoming byte:
		rx_buf[ReceivePtr] = Serial.read();
                ReceivePtr++;
	}
  
  for(i=0;i<ReceivePtr;i++){
      if(rx_buf[i]>0xE0){
        tmp =ReceivePtr-i;
        switch (rx_buf[i]){
            case 0xE1:
              if(tmp!=8){
                 Serial.println("!Error:date parameter"); 
              }else{
                setDate(tmp-1,i);
                Serial.println("Set Date");
                showTime();
              }
            break;
            case 0xE2:
              showTime();
            break;
            case 0xE3:
              delay(1000);
              readTemp();
            break;
            case 0xF1:
               Serial.print("record times:");
               Serial.println(addr/5,DEC);
            break;
            case 0xF2:
              showRecord();
            break;
            case 0xF3:
              full=false;
              addr=0;
              lastTime=0;
              Serial.println("Start record");
            break;
            case 0xF4:
              if (full)   Serial.println("ReStart record");
              else  {
                saveDate[0]=0xFF;
                saveDate[1]=0xFF;
                saveDate[2]=0xFF;
                saveDate[3]=0xFF;
                saveDate[4]=0xFF;
                writePage(addr, (byte *)saveDate,5);
                Serial.println("Stop record");
              }
              full=!full;              
            break;
            case 0xFA:
              full=true;
              addr=0;
              lastTime=0;
              clearEProm();
            break;
            default:
            break;
        }
        break;
      }
  } 
  clearRx();
  
//===============================================
  
  digitalWrite(ledPin, HIGH);   // sets the LED on
  delay(500);                 
  if(!full)    digitalWrite(ledPin, LOW);   
  delay(500);                 

//-----------------------------------------------
  RTC.get(rtc,true);
  if((lastTime !=rtc[1]) && (!full) ){  //
      lastTime =rtc[1];
      showTime();
      readTemp();
      
     saveDate[0]=rtc[4];  //DATE
     saveDate[1]=rtc[2];  //HOU
     saveDate[2]=rtc[1];  //MIN
     saveDate[3]=Tdata[0];
     saveDate[4]=Tdata[1];
     writePage(addr, (byte *)saveDate, sizeof(saveDate));
     addr+=sizeof(saveDate);
     
     if(addr>4090){
       full=true;
       saveDate[0]=0xFF;
       saveDate[1]=0xFF;
       saveDate[2]=0xFF;
       saveDate[3]=0xFF;
       saveDate[4]=0xFF;
       writePage(addr, (byte *)saveDate,5);
       addr+=3;
     }
  }
}


