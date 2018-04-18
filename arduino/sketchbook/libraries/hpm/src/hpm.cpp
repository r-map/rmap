#include "hpm.h"

hpm::hpm():
	pm25_val(0xFFFF),
	pm10_val(0xFFFF),
	coefficient(0xFF)
{

}

bool hpm::init(Stream *serial){
	_serial = serial;
	_serial->setTimeout(HPM_RESP_TIME);

	// disable autosend by default
	if(stopAutoSend() && stopParticleMeasurement()){
	  Log.notice(F("Initialisation successful" CR));
	    return true;
	}
	Log.notice(F("First initialisation failed" CR));
	if(stopAutoSend() && stopParticleMeasurement()){
	  Log.notice(F("Second initialisation successful" CR));
	    return true;
	}
	Log.error(F("Initialisation failed" CR));
	return false;
}

uint16_t hpm::get(hpm_sensor_type type){
	
  if(type == PM25_TYPE){ 
    uint16_t pm25_valr=pm25_val;
    pm25_val=0xFFFF;
    return pm25_valr;
  }else if(type == PM10_TYPE){
    uint16_t pm10_valr=pm10_val;
    pm10_val=0xFFFF;
    return pm10_valr;
  }
  return 0xFFFF;
}

bool hpm::readParticleMeasuringResults(){

  sendCmd(0x01,0x04);
  pm25_val = pm10_val = 0xFFFF;
  return readResponse();
}

bool hpm::startParticleMeasurement(){
  if (sendCmd(0x01,0x01))
    return readResponse();
  return false;
}

bool hpm::stopParticleMeasurement(){
  if (sendCmd(0x01,0x02))
    return readResponse();
  return false;
}

bool hpm::enableAutoSend(){
  if (sendCmd(0x01,0x40))
    return readResponse();
  return false;
}

bool hpm::stopAutoSend(){
  if (sendCmd(0x01,0x20))
    return readResponse();
  return false;
}

bool hpm::setCustomerAdjustmentCoefficient(uint8_t value){
  if (sendCmd(0x02,0x08,value))
    return readResponse();
  return false;
}

uint8_t hpm::readCustomerAdjustmentCoefficient(){
  if (sendCmd(0x01,0x10)){
    if (readResponse())
      return coefficient;
  }
  return 0xFF;
}

bool hpm::sendCmd(uint8_t len,uint8_t cmd, uint8_t value  ){

  uint8_t cmdBuf[5];
  uint8_t size;
  
  cmdBuf[0] = 0x68;
  cmdBuf[1] = len;
  cmdBuf[2] = cmd;
  if (value > 0){
    cmdBuf[3] = value;
    size=5;
  }else{
    size=4;
  }
  cmdBuf[size-1] = getCheckSum8(cmdBuf, size-1);

  Log.notice(F("Sending command:"));
  for(uint8_t i=0; i< size;i++){
    Log.notice(F(" %X"),cmdBuf[i]);
  }
  Log.notice(F(CR));
  
  flush();
  for(uint8_t i=0; i< size;i++){
    _serial->write(cmdBuf[i]);
  }
  return true;
}

bool hpm::readResponse(){

  uint8_t buf[32];
    
  if (_serial->readBytes(buf, 2) == 2 ){
    
    if(buf[0] == 0xa5 && buf[1] == 0xa5){
      Log.notice(F("Positive ACK" CR));
      return true;
    } else if(buf[0] == 0x96 && buf[1] == 0x96){
      Log.notice(F("Negative ACK" CR));
      return false;
    } else if(buf[0] == 0x40){

      if (_serial->readBytes(buf+2, buf[1]+1) == buf[1]+1){
	uint8_t checksum = buf[buf[1]+2];
	uint8_t chcksum  = getCheckSum8(buf,buf[1]+2 );
	
	if (buf[2] == 0x04){
	
	  if(checksum == chcksum){
	    pm25_val = buf[3] << 8 | buf[4];
	    pm10_val = buf[5] << 8 | buf[6];
	    return true;
	  } else {
	    // if command failed, initialise all values to 0
	    pm25_val = pm10_val = 0xFFFF;
	    Log.error(F("INVALID CHECKSUM" CR));
	    return false;
	  }
	} else if (buf[2] == 0x10){
	  coefficient= buf[3];
	  return true;
	}
      }else{
	Log.notice(F("Timeout" CR));
	return false;
      }
    } else if(buf[0] == 0x42 && buf[1] == 0x4d){ // check if there is auto send buffer
      if (_serial->readBytes(buf+2, 30) == 30){

	for(uint8_t i=0; i< sizeof(buf);i++){
	  // Log.notice HEX(buf[i])
	  // Log.notice(F(" " CR));
	}

	if(getCheckSum(buf, sizeof(buf)) == (buf[30] << 8 | buf[31])){ //checksum
	  //update the values
	  pm25_val = buf[6] << 8 | buf[7];
	  pm10_val = buf[8] << 8 | buf[9];
	  return true;
	}
   
	Log.error(F("Incorrect data/Incorrect checksum/Unknown error" CR));
	  return false;
      }else{
	Log.notice(F("Timeout" CR));
	return false;
      }

    } else {
      Log.notice(F("Unknown ACK %X %X" CR),buf[0],buf[1]);
      return false;
    }
  }
  Log.error(F("Timeout" CR));
  return false;
}

bool hpm::loop(){
	// initialise values to FFFF
	pm25_val = pm10_val = 0xFFFF;
	if(_serial->available()){
	  return readResponse();
	}
	return false;
}

uint8_t hpm::getCheckSum8(uint8_t *buf, uint8_t len){
	uint16_t total = 0;
	for(uint8_t i=0; i < len; i++){
		total += buf[i];
	}
	total = 65535 - total;
	total = total + 1;
	total = total % 256;
	return (uint8_t)total;
}

uint16_t hpm::getCheckSum(uint8_t *buf, uint8_t len){
	uint16_t total = 0;
	for(int i=0 ;i < (len-2); i++){
		total += buf[i];
	}
	return total;
}

void hpm::flush(){
  delay(1);
  while(_serial->available()){
    _serial->read();
  }
}


bool hpm::query_data_auto(unsigned int *pm25, unsigned int *pm10, unsigned int n)
{
    unsigned int pm25_table[n];
    unsigned int pm10_table[n];

    Log.notice(F("HPM query data auto" CR));

    
    for (unsigned int i = 0; i<n; i++) {
      if (!readParticleMeasuringResults()) return false;
      pm25_table[i] = get(PM25_TYPE);
      if (pm25_table[i] == 0xFFFF) return false;
      pm10_table[i] = get(PM10_TYPE);
      if (pm10_table[i] == 0xFFFF) return false;

      Log.notice(F("PM25 %d" CR),pm25_table[i]);
      Log.notice(F("PM10 %d" CR),pm10_table[i]);
      
      //recommended query interval of not less than 10 seconds

      /*
	https://forum.digikey.com/t/hpm-series-pm2-5-particle-sensor/858
	How fast does the HPM Series analyze media and respond?

	A. Ultra-fast, the HPM Series analyzes media in less than six seconds.
	This speed allows the HPM Series to quickly analyze and provide data to supporting equipment,
	allowing the device to respond to changing conditions in real-time.
	It was suggested while we were in training to allow the unit to run for 15 seconds
	to ensure that you see a normalized result. The output of the sensor is a 10 second average.
      */
      
      if (i < (n-1)) delay(10000);
    }

    _filter_data(n, pm25_table, pm10_table, pm25, pm10);

    return true;
}


void hpm::_filter_data(unsigned int n, unsigned int *pm25_table, unsigned int *pm10_table, unsigned int *pm25, unsigned int *pm10)
{
    unsigned int pm25_min, pm25_max, pm10_min, pm10_max, pm25_sum, pm10_sum;

    pm10_sum = pm10_min = pm10_max = pm10_table[0];
    pm25_sum = pm25_min = pm25_max = pm25_table[0];

    for (int i=1; i<n; i++) {
        if (pm10_table[i] < pm10_min) {
            pm10_min = pm10_table[i];
        }
        if (pm10_table[i] > pm10_max) {
            pm10_max = pm10_table[i];
        }
        if (pm25_table[i] < pm25_min) {
            pm25_min = pm25_table[i];
        }
        if (pm25_table[i] > pm25_max) {
            pm25_max = pm25_table[i];
        }
        pm10_sum += pm10_table[i];
        pm25_sum += pm25_table[i];
    }

    if (n > 2) {
        *pm10 = (pm10_sum - pm10_max - pm10_min)/(n-2);
        *pm25 = (pm25_sum - pm25_max - pm25_min)/(n-2);
    } else if (n > 1) {
        *pm10 = (pm10_sum - pm10_min)/(n-1);
        *pm25 = (pm25_sum - pm25_min)/(n-1);
    } else {
        *pm10 = pm10_sum/n;
        *pm25 = pm25_sum/n;
    }
}

