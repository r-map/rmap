#include <WProgram.h>
#include <Wire.h>
#include <DS1307.h>

int rtc[7];
byte rr[7];
int ledPin =  13;
void setup()
{
  DDRC|=_BV(2) |_BV(3);  // POWER:Vcc Gnd
  PORTC |=_BV(3);  // VCC PINC3
  pinMode(ledPin, OUTPUT);  
  Serial.begin(9600);
  RTC.get(rtc,true);
  if(rtc[6]<12){
    RTC.stop();
    RTC.set(DS1307_SEC,1);
    RTC.set(DS1307_MIN,27);
    RTC.set(DS1307_HR,01);
    RTC.set(DS1307_DOW,7);
    RTC.set(DS1307_DATE,12);
    RTC.set(DS1307_MTH,2);
    RTC.set(DS1307_YR,12);
    RTC.start();
  }
  //RTC.SetOutput(LOW);
  //RTC.SetOutput(HIGH);
  //RTC.SetOutput(DS1307_SQW1HZ);
  //RTC.SetOutput(DS1307_SQW4KHZ);
  //RTC.SetOutput(DS1307_SQW8KHZ);
  RTC.SetOutput(DS1307_SQW32KHZ);
}

void loop()
{
  int i;
  RTC.get(rtc,true);

  for(i=0; i<7; i++)
  {
    Serial.print(rtc[i]);
    Serial.print(" ");
  }
  Serial.println();
	digitalWrite(ledPin, HIGH); 
	delay(500);
	digitalWrite(ledPin, LOW);
	delay(500);
 if (Serial.available() > 6) {
     for(i=0;i<7;i++){
       rr[i]=BCD2DEC(Serial.read());
     }
     Serial.println("SET TIME:");
       RTC.stop();
    RTC.set(DS1307_SEC,rr[6]);
    RTC.set(DS1307_MIN,rr[5]);
    RTC.set(DS1307_HR,rr[4]);
    RTC.set(DS1307_DOW,rr[3]);
    RTC.set(DS1307_DATE,rr[2]);
    RTC.set(DS1307_MTH,rr[1]);
    RTC.set(DS1307_YR,rr[0]);
    RTC.start();
 }
}
char BCD2DEC(char var){
  if (var>9){
     var=(var>>4)*10+(var&0x0f);
  }
  return var;
}
