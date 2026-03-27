#include "common.h"

/*
  Documentation at
  https://www.sigmaelectronica.net/wp-content/uploads/2024/07/quectel_lc26gablc76g_series_i2c_application_note_v1-0.pdf
*/

int gpsI2cThread::i2c_write(uint8_t device_addr, const uint8_t* writeData, size_t data_len) {

  LockGuard guard(*data->i2cmutex);

  Wire.beginTransmission(device_addr);
  Wire.write(writeData, data_len);
  return Wire.endTransmission();
}

int gpsI2cThread::i2c_read(uint8_t device_addr, char* readData, size_t data_len) {
  int RxIdx = 0;

  LockGuard guard(*data->i2cmutex);
  
  size_t returned = Wire.requestFrom(device_addr, data_len);
  if (returned == data_len){
    while(Wire.available()) {
      readData[RxIdx++] = Wire.read();  // Receive a byte & push it into the RxBuffer
    }
    return 0;	
  }else{
    return 1;
  }
}

void gpsI2cThread::reset(){
  delay(12);
 
  uint8_t none;
  //i2c_write(DEVICE_ADDRESS, &none, sizeof(none));
  //i2c_write(DEVICE_ADDRESS_R, &none, sizeof(none));
  i2c_write(DEVICE_ADDRESS, &none, 0);
  i2c_write(DEVICE_ADDRESS_R, &none, 0);
  dataLength = 0;  
}

void gpsI2cThread::stepOne() {
  uint8_t writeData[] = { 0x08, 0x00, 0x51, 0xAA, 0x04, 0x00, 0x00, 0x00 };

  // A
  if (i2c_write(DEVICE_ADDRESS, writeData, sizeof(writeData)) != 0) {
    data->logger->error(F("gps Failed to write data step one A"));
    reset();
    return;
  }
  delay(12);

  // B
  char readData[4] = { 0 };

  if (i2c_read(DEVICE_ADDRESS_R, readData, sizeof(readData)) != 0) {
    data->logger->error(F("gps Failed to read data step one B"));
    reset();
    return;
  }

  dataLength = (readData[0]) | (readData[1] << 8) | (readData[2] << 16) | (readData[3] << 24);
  data->logger->trace(F("gps Data length to read one: %l"), dataLength);
}

size_t gpsI2cThread::stepTwo(char* dataBuffer,size_t dataBufferLen) {

  if (dataLength == 0) {
    return 0;
  }

  // A
  uint8_t writeData[] = { 0x00, 0x20, 0x51, 0xAA };

  if (dataBufferLen >= dataLength) {
    dataBufferLen=dataLength;
    dataLength=0; // terminate read loop
  } else {
    dataLength-=dataBufferLen;
  }
  uint8_t readData[4];
  readData[0]=(dataBufferLen & 0xFF);
  readData[1]=(dataBufferLen >> 8 & 0xFF);
  readData[2]=(dataBufferLen >> 16 & 0xFF);
  readData[3]=(dataBufferLen >> 24 & 0xFF);

  data->logger->trace(F("gps Data length to read two: %l"), dataBufferLen);
  
  uint8_t dataToSend[sizeof(writeData) + sizeof(readData)];
  memcpy(dataToSend, writeData, sizeof(writeData));
  memcpy(dataToSend + sizeof(writeData), readData, sizeof(readData));
  delay(12);

  if (i2c_write(DEVICE_ADDRESS, dataToSend, sizeof(dataToSend)) != 0) {
    data->logger->error(F("gps Failed to write step two A"));
    reset();
    return 0;
  }

  delay(12);

  // B
  if (i2c_read(DEVICE_ADDRESS_R, dataBuffer, dataBufferLen) != 0) {
    data->logger->error(F("gps Failed to read data step two B"));
    reset();
    return 0;
  }

  return dataBufferLen;
  //for (uint32_t i = 0; i < dataBufferLen; i++) {
  //  data->logger->trace(F("%s"),(char)dataBuffer[i]);
  //}
}

void gpsI2cThread::doI2cNmea(){

  memset(&mgps,0,sizeof(mgps));

  # define MESSAGELEN (33)     // 32 bytes + teminator
  char message[MESSAGELEN];
  uint8_t len;
  
  stepOne();
  if (dataLength == 0){
    nomessagecounter++;
    if (nomessagecounter > 5){
      data->logger->error(F("gps Failed to read data; reset"));
      reset();
      nomessagecounter=0;
    }
    return;
  }
  nomessagecounter=0;
  while (dataLength != 0) {
    size_t len=stepTwo(message,MESSAGELEN-1);  
    message[len]=0;     // add terminator for printing

    if (len > 0) data->logger->trace(F("gps message: %s"), message);

    for (uint8_t i = 0; i < len; i++) {
      uint8_t gpsflag;
      //Serial.print(message[i],DEC);
      //Serial.println(" ");
      gpsflag = gps.encode(message[i]);

      if (gps.valid){
	data->logger->notice(F("gps RMC latitude : %5"), mgps.rmc.dms.latitude);
	data->logger->notice(F("gps RMC longitude: %5"), mgps.rmc.dms.longitude);
	//data->logger->notice(F("gps GGA latitude : %5"), mgps.gga.dms.latitude);
	//data->logger->notice(F("gps GGA longitude: %5"), mgps.gga.dms.longitude);
	//data->logger->notice(F("gps GLL latitude : %5"), mgps.gll.dms.latitude);
	//data->logger->notice(F("gps GLL longitude: %5"), mgps.gll.dms.longitude);
	data->logger->notice(F("gps RMC datetime: %d %d %d %d %d %d"), mgps.rmc.time.year, mgps.rmc.time.mon, mgps.rmc.time.day,
			     mgps.rmc.time.hours, mgps.rmc.time.min, mgps.rmc.time.sec);  
	
	setTime(mgps.rmc.time.hours, mgps.rmc.time.min, mgps.rmc.time.sec
		,mgps.rmc.time.day,mgps.rmc.time.mon,mgps.rmc.time.year);
	if (!data->frtosRTC->set(now())){
	  frtosLog.error("gps Setting RTC time from GPS!");
	}
	
	/*
	  If you just need “GPS” coordinates, any of the GGA, RMC, or GLL
	  sentences will do the job. However, if you need specific
	  information, like an object’s altitude, you will need to use GGA
	  sentences. Similar to this, if you need information about the
	  object’s speed, you will have to use RMC sentences. That said,
	  NMEA sentences should be chosen according to the additional
	  information that you need.
	*/
	
	data->georef->mutex->Lock();    
	itoa(int(std::round(mgps.rmc.dms.latitude*100000)),data->georef->lat,10);
	itoa(int(std::round(mgps.rmc.dms.longitude*100000)),data->georef->lon,10);
	data->georef->timestamp=now();           // TODO create datetime from RMC datetime
	data->georef->mutex->Unlock();
	timestamp=now();
	data->status->receive=ok;
      }
    }
  }
}

using namespace cpp_freertos;

gpsI2cThread::gpsI2cThread(gps_i2c_data_t* gps_data)
  : Thread{"GPS", TASK_GPS_STACK_SIZE, TASK_GPS_PRIORITY
           # if portNUM_PROCESSORS > 1
	   ,1  // if multicore 1 indicate the index number of the CPU which the task should be pinned to
           #endif
          }
    ,data{gps_data}
{
  //data->logger->notice(F("gps Create Thread %s %d"), GetName().c_str(), data->id);
  data->status->receive=unknown;
  data->status->memory_collision=ok;
  data->status->no_heap_memory=ok;
  timestamp=0;
  dataLength=0;
  nomessagecounter=0;
  //Start();
};

gpsI2cThread::~gpsI2cThread()
{
}
  
void gpsI2cThread::Cleanup()
{
  data->logger->notice(F("Delete Thread %s %d"), GetName().c_str(), data->id);
  data->status->receive=unknown;
  data->status->memory_collision=unknown;
  data->status->no_heap_memory=unknown;
  delete this;
}

void gpsI2cThread::Run() {
  data->logger->notice(F("Starting Thread %s %d"), GetName().c_str(), data->id);

  gps.init(&mgps);
  //gps.set_filter(0xE); // "RMC","GGA","GLL"
  gps.set_filter(0x2); // "RMC"

  reset();
  data->logger->notice(F("gps Listening on GPS I2C addesses"));
  
  for(;;){

    doI2cNmea();

    if ((now()-timestamp) > 30){
      data->status->receive=error;
    }else{
      data->status->receive=ok;
    }
    Delay(500);
    //Delay(Ticks::SecondsToTicks(1));
    //if( esp_get_minimum_free_heap_size() < HEAP_MIN_WARNING){
    //  data->logger->error(F("HEAP: %l"),esp_get_minimum_free_heap_size());
    //  data->status->no_heap_memory=error;
    //}
    
    //data->logger->notice("stack gps: %d",uxTaskGetStackHighWaterMark(NULL));
    if(uxTaskGetStackHighWaterMark(NULL) < STACK_MIN_WARNING){
      data->logger->error(F("gps stack"));
      data->status->memory_collision=error;
    }
  }
}

