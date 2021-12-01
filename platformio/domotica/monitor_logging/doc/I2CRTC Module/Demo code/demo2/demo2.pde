#include <OneWire.h>
#include <WProgram.h>
/* DS18S20 Temperature chip i/o */
#include <Wire.h>
#include <DS1307.h>

OneWire  ds(2);  // on pin 2
byte Tdata[12];
int rtc[7];
//int deviceaddress = 0x50;
int deviceaddress = 0x52;
int addr=0; //first address
int lastTime;
byte saveDate[6];
boolean full=false;
int ledPin = 13; 
int outpin=3;
int val = 0;     // variable to store the read value


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

void rdBuffer(void) {
    byte Date[10]={0,0,0,0,0,0,0,0,0,0};
    Wire.beginTransmission(deviceaddress);
    Wire.send((byte)(0 >> 8)); // MSB
    Wire.send((byte)(0 & 0xFF)); // LSB
    Wire.endTransmission();
    Wire.requestFrom(deviceaddress,10);
    int c = 0;
    Serial.println("Show Record:");
    for ( c = 0; c < 10; c++ ){
      if (Wire.available()) Date[c] = Wire.receive();
       Serial.print(Date[c],DEC);
       Serial.print(" ");
    }
    Serial.println(" ");
}

void showRecord(void){
  byte i=0;
  byte Date[30];
 Serial.println("Show Record:");
 readBuffer(0, (byte *)Date, 30);
    for(i=0;i<30;i++){
        Serial.print(Date[i],DEC);
        Serial.print(" ");
    }
}
void showStr(void){
  byte i=0;
  byte Date[10]={0,0,0,0,0,0,0,0,0,0};
 Serial.println("Show Record:");
 readBuffer(0, (byte *)Date, 10);
    for(i=0;i<10;i++){
        Serial.print(Date[i],HEX);
        Serial.print(" ");
    }
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
    delay(20);
    digitalWrite(ledPin, HIGH);  
    delay(20);
  }
   Serial.println("EEPRom Clear Done");
}


void DS1302_SetOut(byte data ) {
    Wire.beginTransmission(B1101000);
    Wire.send(7); // LSB
    Wire.send(data);
    Wire.endTransmission();
}
byte DS1302_GetOut(void) {
    byte rdata = 0xFF;
    Wire.beginTransmission(B1101000);
    Wire.send(7); // LSB
    Wire.endTransmission();
    Wire.requestFrom(B1101000,1);
    if (Wire.available()) {
      rdata = Wire.receive();
      Serial.println(rdata,HEX);
    }
    return rdata;
}
void setup() 
{
  Serial.begin(9600);
  DDRC|=_BV(2) |_BV(3);
  PORTC |=_BV(3);
  DS1302_SetOut(0x00);
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
    pinMode(outpin,INPUT);
    pinMode(4,OUTPUT);
     digitalWrite(4, LOW); 
}

void loop() 
{
  byte i;
  byte present = 0;
  unsigned int Temper=0;
  float TT=0.0;
  byte incomingByte =0;
  
  if (Serial.available() > 0) {
     incomingByte  = Serial.read();
     if(incomingByte==0xff){
         DS1302_GetOut();
     }else if(incomingByte==0xfe){
        DS1302_SetOut(incomingByte);
        DS1302_GetOut();
     //-----------------------------------------------
     }else if(incomingByte==0xE0){	//������I2C
		Wire.begin();
		Serial.println("Reset I2C ");
     }else if(incomingByte==0xE1){	//����ʱ��
		RTC.stop();
		RTC.set(DS1307_SEC,1);
		RTC.set(DS1307_MIN,52);
		RTC.set(DS1307_HR,16);
		RTC.set(DS1307_DOW,2);
		RTC.set(DS1307_DATE,25);
		RTC.set(DS1307_MTH,1);
		RTC.set(DS1307_YR,11);
		RTC.start();
		DS1302_SetOut(0x00);
		Serial.println("Reset Time ");
     }else if(incomingByte==0xE2){	//��ȡʱ��
		RTC.get(rtc,true);
		Serial.println(" ");
		Serial.print("Time=");
		for(int i=0; i<7; i++)        {
          Serial.print(rtc[i]);
          Serial.print(" ");
        }
     //-----------------------------------------------
     }else if(incomingByte==0xD0){	//���EPROM
		clearEProm();
		Serial.println("CLEAR EEPROM");
     }else if(incomingByte==0xD1){	//д��EPROM
		 saveDate[0]=0x00;  //DATE
		 saveDate[1]=0x01;  //HOU
		 saveDate[2]=0x02;  //MIN
		 saveDate[3]=0x03;  //SEC
		 saveDate[4]=0x04;
		 saveDate[5]=0x05;
		 writePage(0X0000, (byte *)saveDate, 6);
		 Serial.println("WRITE EEPROM");
     }else if(incomingByte==0xD2){	//��ȡEPROM
		rdBuffer();
     //-----------------------------------------------
	 }else if(incomingByte==0xC0){	//��ȡ�¶�
		ds.reset();
		ds.write(0xCC,1);
		ds.write(0x44,1);         // start conversion, with parasite power on at the end
		delay(1000);
		present = ds.reset();
		ds.write(0xCC,1);    
		ds.write(0xBE);         // Read Scratchpad
		Serial.println(" ");
		Serial.print(present,HEX);
       for ( i = 0; i < 9; i++) {           // we need 9 bytes
          Tdata[i] = ds.read();
        }
        Serial.print("\t Temperature=");
        Temper = (Tdata[1]<<8 | Tdata[0]);
        TT =Temper*0.0625;
        //12	11	10	9
        //750       375     187.5   93.75
        //0.0625	0.125	0.25	0.5	
        Serial.println(TT);
     }
  }
  
  
  digitalWrite(ledPin, HIGH);   // sets the LED on
  delay(500);                 
  if(!full)    digitalWrite(ledPin, LOW);   
  delay(500);                 
   val = digitalRead(outpin);   // read the
  
}


