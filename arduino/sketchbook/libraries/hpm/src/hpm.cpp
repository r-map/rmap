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

  flush();
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

  Log.notice(F("Sending command" CR));
    for(uint8_t i=0; i< size;i++){
      //Log.debug HEX(cmdBuf[i])
      //Log.debug(F(" " CR));
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
      Log.notice(F("Unknown ACK" CR));
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
	while(_serial->available()){
		_serial->read();
	}
}

