extern "C" {
#include <../Wire/Wire.h>
}
#include "DS1307.h"

DS1307::DS1307()
{
  Wire.begin();
}

DS1307 RTC=DS1307();

// PRIVATE FUNCTIONS

// Aquire data from the RTC chip in BCD format
// refresh the buffer
void DS1307::read(void)
{
  // use the Wire lib to connect to tho rtc
  // reset the resgiter pointer to zero
  Wire.beginTransmission(DS1307_CTRL_ID);
    Wire.send(0x00);
  Wire.endTransmission();

  // request the 7 bytes of data    (secs, min, hr, dow, date. mth, yr)
  Wire.requestFrom(DS1307_CTRL_ID, 7);
  for(int i=0; i<7; i++)
  {
    // store data in raw bcd format
    rtc_bcd[i]=Wire.receive();
  }
}

// update the data on the IC from the bcd formatted data in the buffer
void DS1307::save(void)
{
  Wire.beginTransmission(DS1307_CTRL_ID);
  Wire.send(0x00); // reset register pointer
  for(int i=0; i<7; i++)
  {
    Wire.send(rtc_bcd[i]);
  }
  Wire.endTransmission();
}


// PUBLIC FUNCTIONS
void DS1307::get(int *rtc, boolean refresh)   // Aquire data from buffer and convert to int, refresh buffer if required
{
  if(refresh) read();
  for(int i=0;i<7;i++)  // cycle through each component, create array of data
  {
	rtc[i]=get(i, 0);
  }
}

int DS1307::get(int c, boolean refresh)  // aquire individual RTC item from buffer, return as int, refresh buffer if required
{
  if(refresh) read();
  int v=-1;
  switch(c)
  {
  case DS1307_SEC:
    v=(10*((rtc_bcd[DS1307_SEC] & DS1307_HI_SEC)>>4))+(rtc_bcd[DS1307_SEC] & DS1307_LO_BCD);
	break;
  case DS1307_MIN:
    v=(10*((rtc_bcd[DS1307_MIN] & DS1307_HI_MIN)>>4))+(rtc_bcd[DS1307_MIN] & DS1307_LO_BCD);
	break;
  case DS1307_HR:
    v=(10*((rtc_bcd[DS1307_HR] & DS1307_HI_HR)>>4))+(rtc_bcd[DS1307_HR] & DS1307_LO_BCD);
	break;
  case DS1307_DOW:
    v=rtc_bcd[DS1307_DOW] & DS1307_LO_DOW;
	break;
  case DS1307_DATE:
    v=(10*((rtc_bcd[DS1307_DATE] & DS1307_HI_DATE)>>4))+(rtc_bcd[DS1307_DATE] & DS1307_LO_BCD);
	break;
  case DS1307_MTH:
    v=(10*((rtc_bcd[DS1307_MTH] & DS1307_HI_MTH)>>4))+(rtc_bcd[DS1307_MTH] & DS1307_LO_BCD);
	break;
  case DS1307_YR:
    v=(10*((rtc_bcd[DS1307_YR] & DS1307_HI_YR)>>4))+(rtc_bcd[DS1307_YR] & DS1307_LO_BCD)+DS1307_BASE_YR;
	break;
  } // end switch
  return v;
}

void DS1307::set(int c, int v)  // Update buffer, then update the chip
{
  switch(c)
  {
  case DS1307_SEC:
    if(v<60 && v>-1)
    {
	//preserve existing clock state (running/stopped)
	int state=rtc_bcd[DS1307_SEC] & DS1307_CLOCKHALT;
	rtc_bcd[DS1307_SEC]=state | ((v / 10)<<4) + (v % 10);
    }
    break;
  case DS1307_MIN:
    if(v<60 && v>-1)
    {
	rtc_bcd[DS1307_MIN]=((v / 10)<<4) + (v % 10);
    }
    break;
  case DS1307_HR:
  // TODO : AM/PM  12HR/24HR
    if(v<24 && v>-1)
    {
	rtc_bcd[DS1307_HR]=((v / 10)<<4) + (v % 10);
    }
    break;
  case DS1307_DOW:
    if(v<8 && v>-1)
    {
	rtc_bcd[DS1307_DOW]=v;
    }
    break;
  case DS1307_DATE:
    if(v<31 && v>-1)
    {
	rtc_bcd[DS1307_DATE]=((v / 10)<<4) + (v % 10);
    }
    break;
  case DS1307_MTH:
    if(v<13 && v>-1)
    {
	rtc_bcd[DS1307_MTH]=((v / 10)<<4) + (v % 10);
    }
    break;
  case DS1307_YR:
    if(v<13 && v>-1)
    {
	rtc_bcd[DS1307_YR]=((v / 10)<<4) + (v % 10);
    }
    break;
  } // end switch
  save();
}

void DS1307::stop(void)
{
	// set the ClockHalt bit high to stop the rtc
	// this bit is part of the seconds byte
    rtc_bcd[DS1307_SEC]=rtc_bcd[DS1307_SEC] | DS1307_CLOCKHALT;
    save();
}

void DS1307::start(void)
{
	// unset the ClockHalt bit to start the rtc
	// TODO : preserve existing seconds
    rtc_bcd[DS1307_SEC]=rtc_bcd[DS1307_SEC] & (~DS1307_CLOCKHALT);
	save();
}


void DS1307::SetOutput(int c)  
{
  int out;
  switch(c)
  {
  case HIGH :
	out=DS1307_HIGH_BIT;
  break;
  case LOW :
	out=DS1307_LOW_BIT;
  break;
  case DS1307_SQW1HZ :
	out=DS1307_SQW1HZ_BIT;
  break;
  case DS1307_SQW4KHZ :
	out=DS1307_SQW4KHZ_BIT;
  break;
  case DS1307_SQW8KHZ :
	out=DS1307_SQW8KHZ_BIT;
  break;
  case DS1307_SQW32KHZ :
	out=DS1307_SQW32KHZ_BIT;
  break;
  default: 
	out=DS1307_LOW_BIT;
  }
  
  Wire.beginTransmission(DS1307_CTRL_ID);
  Wire.send(0x07); 
  Wire.send(out);
  Wire.endTransmission();
}

int DS1307::GetOutput(void)  
{
  Wire.beginTransmission(DS1307_CTRL_ID);
  Wire.send(0x07);
  Wire.endTransmission();

  Wire.requestFrom(DS1307_CTRL_ID, 1);
  int out=Wire.receive();
  
  int c=-1;
  switch(out)
  {
  case DS1307_HIGH_BIT :
	c=HIGH;
  break;
  case DS1307_LOW_BIT :
	c=LOW;
  break;
  case DS1307_SQW1HZ_BIT :
	c=DS1307_SQW1HZ;
  break;
  case DS1307_SQW4KHZ_BIT :
	c=DS1307_SQW4KHZ;
  break;
  case DS1307_SQW8KHZ_BIT :
	c=DS1307_SQW8KHZ;
  break;
  case DS1307_SQW32KHZ_BIT :
	c=DS1307_SQW32KHZ;
  break;
  }
  return c;
}