#include "Wire.h"      // Wire (I2C) defines
#include "SensorDriver.h"

//void SensorDriverInit()
//{
//  Wire.begin();                // join i2c bus as master
//}


int THcounter=0;
int SDS011counter=0;

SensorDriver* SensorDriver::create(const char* driver,const char* type) {

  IF_SDSDEBUG(SDDBGSERIAL.print(F("#NEW driver: ")));
  IF_SDSDEBUG(SDDBGSERIAL.print(driver);SDDBGSERIAL.println(type));

  if (strcmp(driver, "I2C") == 0)
    {
#if defined (TMPDRIVER)
      if (strcmp(type, "TMP") == 0)
	return new SensorDriverTmp();
      else 
#endif
#if defined (ADTDRIVER)
      if (strcmp(type, "ADT") == 0)
	return new SensorDriverAdt7420();
      else 
#endif
#if defined (HIHDRIVER)
      if (strcmp(type, "HIH") == 0)
	return new SensorDriverHih6100();
      else
#endif
#if defined (HYTDRIVER)
      if (strcmp(type, "HYT") == 0)
	return new SensorDriverHyt271();
      else
#endif
#if defined (BMPDRIVER)
      if (strcmp(type, "BMP") == 0)
	return new SensorDriverBmp085();
      else
#endif
#if defined (DAVISWIND1)
      if (strcmp(type, "DW1") == 0)
	return new SensorDriverDw1();
      else
#endif
#if defined (TIPPINGBUCKETRAINGAUGE)
      if (strcmp(type, "TBR") == 0)
	return new SensorDriverTbr();
      else
#endif
#if defined (TEMPERATUREHUMIDITY_ONESHOT)
      if (strcmp(type, "STH") == 0)
	return new SensorDriverTHoneshot();
      else
#endif

#if defined (TEMPERATUREHUMIDITY_REPORT)
      if (strcmp(type, "ITH") == 0)   // istantaneous
	return new SensorDriverTH60mean();
      else
      if (strcmp(type, "MTH") == 0)   // mean
	return new SensorDriverTHmean();
      else
      if (strcmp(type, "NTH") == 0)   // min
	return new SensorDriverTHmin();
      else
      if (strcmp(type, "XTH") == 0)   //max
	return new SensorDriverTHmax();
      else
#endif

#if defined (SDS011_ONESHOT)
      if (strcmp(type, "SSD") == 0)
	return new SensorDriverSDS011oneshot();
      else
#endif
#if defined (SDS011_REPORT)
      if (strcmp(type, "ISD") == 0)   // istantaneous
	return new SensorDriverSDS01160mean();
      else
      if (strcmp(type, "MSD") == 0)   // mean
	return new SensorDriverSDS011mean();
      else
      if (strcmp(type, "NSD") == 0)   // min
	return new SensorDriverSDS011min();
      else
      if (strcmp(type, "XSD") == 0)   //max
	return new SensorDriverSDS011max();
      else
#endif

	return NULL;
    } else {

#if defined (RADIORF24)
  if (strcmp(driver, "RF24") == 0)
    return new SensorDriverRF24();
  else 
#endif
    return NULL;
  }
}
SensorDriver::~SensorDriver() {}

#if defined (RADIORF24)
  #if defined (AES)
void SensorDriver::aes_enc( char* mainbuf, size_t* buflen){
if ((*buflen % 16) != 0) *buflen = (int(*buflen/16)*16) + 16;
  //IF_SDSDEBUG(SDDBGSERIAL.print(F("#encode string  :")));
  //IF_SDSDEBUG(SDDBGSERIAL.print(*buflen));
  //IF_SDSDEBUG(SDDBGSERIAL.println(mainbuf));
  aes128_cbc_enc(_key, _iv, mainbuf, *buflen);
  //IF_SDSDEBUG(SDDBGSERIAL.print(F("#encoded string :")));
  //IF_SDSDEBUG(SDDBGSERIAL.write(mainbuf,*buflen));
  //IF_SDSDEBUG(SDDBGSERIAL.println(F("#")));
}

void SensorDriver::aes_dec( char* mainbuf, size_t* buflen){
if ((*buflen % 16) != 0) *buflen = (int(*buflen/16)*16) + 16;
  //IF_SDSDEBUG(SDDBGSERIAL.print(F("#decode string :")));
  //IF_SDSDEBUG(SDDBGSERIAL.write(mainbuf,*buflen));
  //IF_SDSDEBUG(SDDBGSERIAL.println(F("#")));
  aes128_cbc_dec(_key, _iv, mainbuf, *buflen);
  //IF_SDSDEBUG(mainbuf[*buflen-1]='\0');
  //IF_SDSDEBUG(SDDBGSERIAL.print(F("#decoded string:")));
  //IF_SDSDEBUG(SDDBGSERIAL.println(mainbuf));
}
  #endif

int SensorDriver::setup(const char* driver, const int address, const int node, const char* type, char* mainbuf, size_t lenbuf, RF24Network* network
  #if defined (AES)
			, uint8_t key[] , uint8_t iv[]
  #endif 
)
{
  _node=node;
  _driver=driver;
  _type=type;
  _address = address;
  _mainbuf=mainbuf;
  _lenbuf=lenbuf;
  _network=network;
  #if defined (AES)
  _key=key;
  _iv=iv;
  #endif
  _timing = 0;
  _jsrpcid=0;

  return 0;
}

#else

int SensorDriver::setup(const char* driver, const int address, const int node, const char* type)
{

  _driver=driver;
  _node=node;
  _type=type;
  _address = address;

  _timing = 0;

  return 0;
}

#endif


#if defined (TMPDRIVER)
int SensorDriverTmp::setup(const char* driver, const int address, const int node, const char* type
                      #if defined (RADIORF24)
			   , char* mainbuf, size_t lenbuf
			   , RF24Network* network
                        #if defined (AES)
			   , uint8_t key[] , uint8_t iv[]
                        #endif
                      #endif
			     )
{

  SensorDriver::setup(driver,address,node,type
                      #if defined (RADIORF24)
			   , mainbuf, lenbuf, network
                        #if defined (AES)
			   , key,iv
                        #endif
                      #endif
			   );

  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write((byte)0x01);            // Set the register pointer to (0x01)
  Wire.write((byte)0xE1);            // Set resolution and SHUTDOWN MODE and one shot
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  
  return SD_SUCCESS;
  
}

int SensorDriverTmp::prepare(unsigned long& waittime)
{

  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write((byte)0x01);            // Set the register pointer to (0x01)
  Wire.write((byte)0xE1);            // Set resolution and SHUTDOWN MODE and one shot
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  _timing=millis();
  waittime= 500ul;

  return SD_SUCCESS;

}

int SensorDriverTmp::get(long values[],size_t lenvalues)
{

  if (millis() - _timing > MAXDELAYFORREAD)     return SD_INTERNAL_ERROR;

  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write((byte)0x00);             // Set the register pointer to (0x00)
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  Wire.requestFrom(_address,2);

  if (Wire.available() < 2)    // slave may send less than requested
  { 
    return SD_INTERNAL_ERROR;
  }
  byte MSB = Wire.read();
  byte LSB = Wire.read();

  if ((MSB == 255) & (LSB ==255))
  { 
    return SD_INTERNAL_ERROR;
  }

  //it's a 12bit int, using two's compliment for negative
  int TemperatureSum = ((MSB << 8) | LSB) >> 4 & 0xFFF; 
  //int TemperatureSum = ((MSB << 8) | LSB) >> 4 ; 

  if (TemperatureSum & 0x800)
  {
    TemperatureSum=TemperatureSum - 0x1000;
  }
  if (lenvalues >= 1)  values[0] = (long)(TemperatureSum*6.25 + 27315.) ;

  _timing=0;

  return SD_SUCCESS;

}

  #if defined(USEAJSON)
aJsonObject* SensorDriverTmp::getJson()
{
  long values[1];
  aJsonObject* jsonvalues;
  jsonvalues = aJson.createObject();
  if (SensorDriverTmp::get(values,1) == SD_SUCCESS){
    aJson.addNumberToObject(jsonvalues, "B12101", values[0]);      
    // if you have a second value add here
    //aJson.addNumberToObject(jsonvalues, "B12102", values2);      

  }else{
    aJson.addNullToObject(jsonvalues, "B12101");
    // if you have a second value add here
    //aJson.addNullToObject(jsonvalues, "B12102");
  }
  return jsonvalues;
}
  #endif
#endif

#if defined (ADTDRIVER)
int SensorDriverAdt7420::setup(const char* driver, const int address, const int node, const char* type
                         #if defined (RADIORF24)
			       , char* mainbuf, size_t lenbuf, RF24Network* network
                           #if defined (AES)
			       , uint8_t key[] , uint8_t iv[]
                           #endif 
                         #endif
			       )
{

  SensorDriver::setup(driver,address,node,type
                  #if defined (RADIORF24)
		      ,mainbuf,lenbuf,network
                    #if defined (AES)
		      , key,iv
                    #endif
                  #endif
		      );
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write((byte)0x03);                              // Set the register pointer to (0x01)
  Wire.write((byte)0x20);                              // Set resolution and one shot
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  return SD_SUCCESS;

}

int SensorDriverAdt7420::prepare(unsigned long& waittime)
{

  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write((byte)0x03);                              // Set the register pointer to (0x01)
  Wire.write((byte)0x20);                              // Set resolution and one shot
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  waittime=250ul;
  _timing=millis();

  return SD_SUCCESS;

}

int SensorDriverAdt7420::get(long values[],size_t lenvalues)
{
  if (millis() - _timing > MAXDELAYFORREAD)     return SD_INTERNAL_ERROR;

  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write((byte)0x00);                              // Set the register pointer to (0x00)
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  Wire.requestFrom(_address,2);

  if (Wire.available() < 2)    // slave may send less than requested
  { 
    return SD_INTERNAL_ERROR;
  }
  byte MSB = Wire.read();
  byte LSB = Wire.read();

  if ((MSB == 255) & (LSB ==255))
  { 
    return SD_INTERNAL_ERROR;
  }

  //it's a 13bit int, using two's compliment for negative
  int TemperatureSum = ((MSB << 8) | LSB) >> 3 & 0xFFF; 
  //int TemperatureSum = ((MSB << 8) | LSB) >> 3 ; 

  if (TemperatureSum & 0x800)
  {
    TemperatureSum=TemperatureSum - 0x1000;
  }

  if (lenvalues >= 1)  values[0] = (long)(TemperatureSum*6.25 + 27315.) ;
  _timing=0;

  return SD_SUCCESS;

}

  #if defined(USEAJSON)
aJsonObject* SensorDriverAdt7420::getJson()
{
  long values[1];
  aJsonObject* jsonvalues;
  jsonvalues = aJson.createObject();
  
  if (SensorDriverAdt7420::get(values,1) == SD_SUCCESS){
    aJson.addNumberToObject(jsonvalues, "B12101", values[0]);      
    // if you have a second value add here
    //aJson.addNumberToObject(jsonvalues, "B12102", values2);      

  }else{
    aJson.addNullToObject(jsonvalues, "B12101");
    // if you have a second value add here
    //aJson.addNullToObject(jsonvalues, "B12102");
  }
  return jsonvalues;
}
  #endif
#endif

#if defined (RADIORF24)

int SensorDriverRF24::setup(const char* driver, const int address, const int node, const char* type
			    , char* mainbuf, size_t lenbuf, RF24Network* network
                         #if defined (AES)
			    , uint8_t key[] , uint8_t iv[]
                         #endif
			    )
{
  // setup for remote sensor whould be done local (where you have sensors hard connected !)
  SensorDriver::setup(driver,address,node,type, mainbuf, lenbuf, network
                 #if defined (AES)
			, key,iv
                 #endif
		      );
  return SD_SUCCESS;  // do nothing
}

int SensorDriverRF24::prepare(unsigned long& waittime)
{

   IF_SDSDEBUG(SDDBGSERIAL.println(F("#Radio Sending... prepare")));

  // Pump the network regularly
  _network->update();

  RF24NetworkHeader header(_node,0);
  // example calling rpc:
  // {"jsonrpc": "2.0", "method": "prepare", "params": {"driver":"TMP","node":1,"type":"TMP","address": 72}, "id": 0}

  aJsonObject *rpc = aJson.createObject();
  aJson.addStringToObject(rpc, "jsonrpc", "2.0");
  aJson.addStringToObject(rpc, "method", "prepare");
  aJsonObject *params = aJson.createObject();
  aJson.addNumberToObject(params, "node", _node);
  // on the remote node we change driver to local (i2c)
  //aJson.addStringToObject(params, "driver","I2C");
  aJson.addStringToObject(params, "driver", _driver);
  aJson.addStringToObject(params, "type", _type);
  aJson.addNumberToObject(params, "address", _address);
  aJson.addItemToObject(rpc, "params", params);
  aJson.addNumberToObject(rpc, "id", _jsrpcid);
  aJson.print(rpc,_mainbuf, _lenbuf);
  aJson.deleteItem(rpc);

  _jsrpcid++;

  //strcpy(_mainbuf,"{\"jsonrpc\": \"2.0\", \"method\": \"prepare\", \"params\": {\"type\":\"TMP\",\"address\": 72}, \"id\": 0}");

  size_t buflen=strlen(_mainbuf)+1;
  #if defined (AES)
  SensorDriver::aes_enc(_mainbuf, &buflen);
  #endif
  bool ok = _network->write(header,_mainbuf,buflen);

  if (!ok){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#radio prepare failed.")));
    return SD_INTERNAL_ERROR;             // End Write Transmission 
  }

  unsigned long start_at = millis();

  while (true){
    _network->update();
    if (_network->available())              break;
    if ( ( millis() - start_at) > 500 ) break;
  }

  size_t size = _network->read(header,_mainbuf,_lenbuf);
  // manage json rpc messages
  if (size <= 0){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#error getting rf24 response")));
    return SD_INTERNAL_ERROR;
  }

  #if defined (AES)
  SensorDriver::aes_dec(_mainbuf, &size);
  #endif

  _mainbuf[size-1]='\0';
  IF_SDSDEBUG(SDDBGSERIAL.print(F("#receive: ")));
  IF_SDSDEBUG(SDDBGSERIAL.println(_mainbuf));
  
  aJsonObject *nodemsg = aJson.parse(_mainbuf);
  
  if (!nodemsg){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#error getting json")));
    aJson.deleteItem(nodemsg);
    return SD_INTERNAL_ERROR;
  }

  // parse rpc information
  aJsonObject* id = aJson.getObjectItem(nodemsg, "id");
  if (!id){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#id not found")));
    aJson.deleteItem(nodemsg);
    return SD_INTERNAL_ERROR;
  }
    
  aJsonObject* jsonrpc = aJson.getObjectItem(nodemsg, "jsonrpc");
  if (!jsonrpc){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#jsonrpc not found")));
    aJson.deleteItem(nodemsg);
    return SD_INTERNAL_ERROR;
  }

  if (strcmp (jsonrpc->valuestring,"2.0" ) != 0) {
    IF_SDSDEBUG(SDDBGSERIAL.print(F("#jsonrpc version is wrong:")));
    IF_SDSDEBUG(SDDBGSERIAL.println(jsonrpc->valuestring));
    aJson.deleteItem(nodemsg);
    return SD_INTERNAL_ERROR;
  }

  // manage good message
  IF_SDSDEBUG(SDDBGSERIAL.println(F("#its a valid response")));
  aJsonObject* waittimeo = aJson.getObjectItem(nodemsg, "result");
  if (!waittimeo){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#result not found")));
    aJson.deleteItem(nodemsg);
    return SD_INTERNAL_ERROR;
  }
  
  aJsonObject* waittimei = aJson.getObjectItem(waittimeo,"waittime");
  if (!waittimei) {
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#response not found")));
    aJson.deleteItem(nodemsg);
    return SD_INTERNAL_ERROR;
  }

  //waittime=250ul;
  //waittime=(unsigned long)waittimei->valueint;
  waittime=waittimei->valueint;
  IF_SDSDEBUG(SDDBGSERIAL.print(F("#waittime: ")));
  IF_SDSDEBUG(SDDBGSERIAL.println(waittime));
  
  aJson.deleteItem(nodemsg);
  _timing=millis();
  
  return SD_SUCCESS;
}

  #if defined(USEAJSON)

aJsonObject* SensorDriverRF24::getJson()
{
  
  aJsonObject* jsonvalues;

  // Pump the network regularly
  _network->update();
  
  if (millis() - _timing > MAXDELAYFORREAD){
    jsonvalues = aJson.createObject();
    aJson.addNullToObject(jsonvalues, "RF24");
    return jsonvalues; 
  }
  
  IF_SDSDEBUG(SDDBGSERIAL.println(F("#Radio Sending... getjson")));
  
  // example calling rpc:
  // {"jsonrpc": "2.0", "method": "getjson", "params": {"driver":"TMP","type":"TMP","address": 72}, "id": 0}
  // {"jsonrpc": "2.0", "method": "getjson", "params": {"driver":"TMP","type":"ADT","address": 75}, "id": 0}
  
  //strcpy (_mainbuf,"{\"jsonrpc\": \"2.0\", \"method\": \"getvalues\", \"params\": {\"type\":\"TMP\",\"address\": 72}, \"id\": 0}");
  // mmmmm strcpy (_mainbuf,"{\"jsonrpc\": \"2.0\", \"method\": \"getvalues\", \"params\": {}, \"id\": 0}");

  aJsonObject *rpc = aJson.createObject();
  aJson.addStringToObject(rpc, "jsonrpc", "2.0");
  aJson.addStringToObject(rpc, "method", "getjson");
  aJsonObject *params = aJson.createObject();
  aJson.addNumberToObject(params, "node", _node);
  // on the remote node we change driver to local (i2c)
  //aJson.addStringToObject(params, "driver", "I2C");
  aJson.addStringToObject(params, "driver", _driver);
  aJson.addStringToObject(params, "type", _type);
  aJson.addNumberToObject(params, "address", _address);
  aJson.addItemToObject(rpc, "params", params);
  aJson.addNumberToObject(rpc, "id", _jsrpcid);
  aJson.print(rpc,_mainbuf, _lenbuf);
  aJson.deleteItem(rpc);

  RF24NetworkHeader header( _node,0);
  size_t buflen=strlen(_mainbuf)+1;
  #if defined (AES)
  SensorDriver::aes_enc(_mainbuf, &buflen);
  #endif
  bool ok = _network->write(header,_mainbuf,buflen);
  
  if (!ok) {  
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#radio failed.")));
    jsonvalues = aJson.createObject();
    aJson.addNullToObject(jsonvalues, "RF24");
    return jsonvalues; 
  }
  
  unsigned long start_at = millis();

  while (true){
    _network->update();
    if (_network->available())              break;
    if ( ( millis() - start_at) > 500 ) break;
  }

  size_t size = _network->read(header,_mainbuf,_lenbuf);
  
  if (size >0){

    #if defined (AES)
    SensorDriver::aes_dec(_mainbuf, &size);
    #endif

    _mainbuf[size-1]='\0';
    aJsonObject *noderesponse = aJson.parse(_mainbuf);
    jsonvalues = aJson.detachItemFromObject(noderesponse,"result"); 

    _timing=0;
    aJson.deleteItem(noderesponse);
    return jsonvalues;
  // } else{
  //   jsonvalues = aJson.createObject();
  //   aJson.addNullToObject(jsonvalues, "RF24");
  //   aJson.deleteItem(noderesponse);
  //   return jsonvalues; 
  }

  return NULL;
}

 #endif

int SensorDriverRF24::get(long values[], size_t lenvalues)
{
    return SD_INTERNAL_ERROR;
}
    
#endif


#if defined (HIHDRIVER)
int SensorDriverHih6100::setup(const char* driver, const int address, const int node, const char* type
             #if defined (RADIORF24)
			       , char* mainbuf, size_t lenbuf, RF24Network* network
               #if defined (AES)
			       , uint8_t key[] , uint8_t iv[]
               #endif
             #endif
			       )
{

  SensorDriver::setup(driver,address,node,type
             #if defined (RADIORF24)
		      , mainbuf, lenbuf, network
               #if defined (AES)
		      , key,iv
               #endif
             #endif
		      );

  return SD_SUCCESS;

}

int SensorDriverHih6100::prepare(unsigned long& waittime)
{

  Wire.beginTransmission(_address);   // Open I2C line in write mode
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  _timing=millis();
  waittime= 40ul;   //The measurement cycle duration is typically 36.65 ms for temperature and humidity readings

  return SD_SUCCESS;

}

int SensorDriverHih6100::get(long values[],size_t lenvalues)
{

  if (millis() - _timing > MAXDELAYFORREAD)     return SD_INTERNAL_ERROR;

  
  /*   python code
    val = self.bus.read_i2c_block_data(self.address,0,4)
    
    status = (val[0] & 0xc0) >> 6
    humid = float(((val[0] & 0x3f) << 8) + val[1])/0x3ffe*100.
    temp =  float((val[2] << 6)  + ((val[3] & 0xfc) >> 2))/0x3ffe*165.-40.
    
    return {"B12101":nint(temp*100.+27315.),"B13003":nint(humid)}

  */

  uint8_t x, y, s;
  uint16_t h;
  uint16_t t;
  
  Wire.requestFrom(_address, 4);
  if(Wire.available() >= 4) {
    x = Wire.read();
    s = (x & 0xc0 ) >> 6;
    
    switch(s) {
    case 0:
      // status 0 == OK  
      // Normal Operation, Valid Data that has not been fetched since the last measurement cycle

      y = Wire.read();
      //IF_SDSDEBUG(SDDBGSERIAL.print(F("#hih read humidity x,y: ")));
      //IF_SDSDEBUG(SDDBGSERIAL.print(x));
      //IF_SDSDEBUG(SDDBGSERIAL.print(F(" , ")));
      //IF_SDSDEBUG(SDDBGSERIAL.println(y));

      h = (((uint16_t) (x & 0x3f)) << 8) | y;

      x = Wire.read();
      y = Wire.read();
      //IF_SDSDEBUG(SDDBGSERIAL.print(F("#hih read temperature x,y: ")));
      //IF_SDSDEBUG(SDDBGSERIAL.print(x));
      //IF_SDSDEBUG(SDDBGSERIAL.print(F(" , ")));
      //IF_SDSDEBUG(SDDBGSERIAL.println(y));

      t = (((uint16_t) x) << 6) | ((y & 0xfc) >> 2);
      
      //Wire.endTransmission();
      break;

    default:
      // status 1  
      // Stale Data: Data that has already been
      // fetched since the last measurement cycle, or
      // data fetched before the first measurement
      // has been completed

      // status 2
      // Device in Command Mode
      // Command Mode is used for programming the sensor.
      // This mode should not be seen during normal operation

      // status 3
      // Not Used

      return SD_INTERNAL_ERROR;
    }

  }else{
    return SD_INTERNAL_ERROR;
  }

  if (lenvalues >= 1)  values[0] = (long) round (float(h) / 16382. * 100.) ;
  if (lenvalues >= 2)  values[1] = (long) round((float(t) / 16382. * 165. - 40.) * 100. + 27315.) ;
  _timing=0;

  return SD_SUCCESS;

}

#if defined(USEAJSON)
aJsonObject* SensorDriverHih6100::getJson()
{
  long values[2];

  aJsonObject* jsonvalues;
  jsonvalues = aJson.createObject();
  //if (SensorDriverTmp::get2(&humidity,&temperature) == SD_SUCCESS){
  if (SensorDriverHih6100::get(values,2) == SD_SUCCESS){
    aJson.addNumberToObject(jsonvalues, "B13003", values[0]);      

#if defined(SECONDARYPARAMETER)
    // if you have a second value add here
    aJson.addNumberToObject(jsonvalues, "B12101", values[1]);      
#endif
  }else{
    aJson.addNullToObject(jsonvalues, "B12101");
#if defined(SECONDARYPARAMETER)
    // if you have a second value add here
    aJson.addNullToObject(jsonvalues, "B13003");
#endif
  }
  return jsonvalues;
}
#endif
#endif

#if defined (HYTDRIVER)
#include "hyt271.h"
int SensorDriverHyt271::setup(const char* driver, const int address, const int node, const char* type
             #if defined (RADIORF24)
			       , char* mainbuf, size_t lenbuf, RF24Network* network
               #if defined (AES)
			       , uint8_t key[] , uint8_t iv[]
               #endif
             #endif
			       )
{

  SensorDriver::setup(driver,address,node,type
             #if defined (RADIORF24)
		      , mainbuf, lenbuf, network
               #if defined (AES)
		      , key,iv
               #endif
             #endif
		      );

  return SD_SUCCESS;

}

int SensorDriverHyt271::prepare(unsigned long &waittime) {
	Wire.beginTransmission(_address);   	// Open I2C line in write mode
	
	if (Wire.endTransmission())
		return SD_INTERNAL_ERROR;			// End Write Transmission 
	
	_timing = millis();
	waittime = 100ul;   					// The measurement cycle duration is typically 100 ms max for temperature and humidity readings
	
	return SD_SUCCESS;
}

int SensorDriverHyt271::get(long values[], size_t lenvalues) {
	if (millis() - _timing > MAXDELAYFORREAD)
		return SD_INTERNAL_ERROR;
	
	float humidity;
	float temperature;
	
	Wire.requestFrom(_address, I2C_HYT271_READ_HT_DATA_LENGTH);
	
	if (Wire.available() == I2C_HYT271_READ_HT_DATA_LENGTH)
		HYT271_getHT((unsigned long) Wire.read() << 24 | (unsigned long) Wire.read() << 16 | (unsigned long) Wire.read() << 8 | (unsigned long) Wire.read(), &humidity, &temperature);
	else return SD_INTERNAL_ERROR;

	if (lenvalues >= 2) {
		values[0] = (long) round(humidity);
		values[1] = (long) round(temperature*100 + 27315);
	}
	
	_timing = 0;
	return SD_SUCCESS;
}

#if defined(USEAJSON)
aJsonObject* SensorDriverHyt271::getJson() {
	long values[2];
	aJsonObject* jsonvalues;
	jsonvalues = aJson.createObject();
	
	if (SensorDriverHyt271::get(values,2) == SD_SUCCESS) {
		aJson.addNumberToObject(jsonvalues, "B13003", values[0]);
		aJson.addNumberToObject(jsonvalues, "B12101", values[1]);
	}
	else {
		aJson.addNullToObject(jsonvalues, "B12101");
		aJson.addNullToObject(jsonvalues, "B13003");
	}
	
	return jsonvalues;
}
#endif
#endif


#if defined (BMPDRIVER)

/*********************************************************************/

uint8_t SensorDriverBmp085::read8(uint8_t a) {
  uint8_t ret;

  Wire.beginTransmission(_address); // start transmission to device 
#if (ARDUINO >= 100)
  Wire.write(a); // sends register address to read from
#else
  Wire.send(a); // sends register address to read from
#endif
  Wire.endTransmission(); // end transmission
  
  Wire.beginTransmission(_address); // start transmission to device 
  Wire.requestFrom(_address, 1);// send data n-bytes read
#if (ARDUINO >= 100)
  ret = Wire.read(); // receive DATA
#else
  ret = Wire.receive(); // receive DATA
#endif
  Wire.endTransmission(); // end transmission

  return ret;
}

uint16_t SensorDriverBmp085::read16(uint8_t a) {
  uint16_t ret;

  Wire.beginTransmission(_address); // start transmission to device 
#if (ARDUINO >= 100)
  Wire.write(a); // sends register address to read from
#else
  Wire.send(a); // sends register address to read from
#endif
  Wire.endTransmission(); // end transmission
  
  Wire.beginTransmission(_address); // start transmission to device 
  Wire.requestFrom(_address, 2);// send data n-bytes read
#if (ARDUINO >= 100)
  ret = Wire.read(); // receive DATA
  ret <<= 8;
  ret |= Wire.read(); // receive DATA
#else
  ret = Wire.receive(); // receive DATA
  ret <<= 8;
  ret |= Wire.receive(); // receive DATA
#endif
  Wire.endTransmission(); // end transmission

  return ret;
}

void SensorDriverBmp085::write8(uint8_t a, uint8_t d) {
  Wire.beginTransmission(_address); // start transmission to device 
#if (ARDUINO >= 100)
  Wire.write(a); // sends register address to read from
  Wire.write(d);  // write data
#else
  Wire.send(a); // sends register address to read from
  Wire.send(d);  // write data
#endif
  Wire.endTransmission(); // end transmission
}

int32_t SensorDriverBmp085::computeB5(int32_t UT) {
  int32_t X1 = (UT - (int32_t)ac6) * ((int32_t)ac5) >> 15;
  int32_t X2 = ((int32_t)mc << 11) / (X1+(int32_t)md);
  return X1 + X2;
}

uint16_t SensorDriverBmp085::readRawTemperature(void) {
  write8(BMP085_CONTROL, BMP085_READTEMPCMD);
  delay(10);
#if BMP085_DEBUG == 1
  SDDBGSERIAL.print("Raw temp: "); SDDBGSERIAL.println(read16(BMP085_TEMPDATA));
#endif
  return read16(BMP085_TEMPDATA);
}

uint32_t SensorDriverBmp085::readRawPressure(void) {
  uint32_t raw;

  write8(BMP085_CONTROL, BMP085_READPRESSURECMD + (oversampling << 6));

  if (oversampling == BMP085_ULTRALOWPOWER) 
    delay(10);
  else if (oversampling == BMP085_STANDARD) 
    delay(8);
  else if (oversampling == BMP085_HIGHRES) 
    delay(14);
  else 
    delay(26);

  raw = read16(BMP085_PRESSUREDATA);

  raw <<= 8;
  raw |= read8(BMP085_PRESSUREDATA+2);
  raw >>= (8 - oversampling);

 /* this pull broke stuff, look at it later?
  if (oversampling==0) {
    raw <<= 8;
    raw |= read8(BMP085_PRESSUREDATA+2);
    raw >>= (8 - oversampling);
  }
 */

#if BMP085_DEBUG == 1
  SDDBGSERIAL.print("Raw pressure: "); SDDBGSERIAL.println(raw);
#endif
  return raw;
}


int32_t SensorDriverBmp085::readPressure(void) {
  int32_t UT, UP, B3, B5, B6, X1, X2, X3, p;
  uint32_t B4, B7;

  UT = readRawTemperature();
  UP = readRawPressure();

#if BMP085_DEBUG == 1
  // use datasheet numbers!
  UT = 27898;
  UP = 23843;
  ac6 = 23153;
  ac5 = 32757;
  mc = -8711;
  md = 2868;
  b1 = 6190;
  b2 = 4;
  ac3 = -14383;
  ac2 = -72;
  ac1 = 408;
  ac4 = 32741;
  oversampling = 0;
#endif

  B5 = computeB5(UT);

#if BMP085_DEBUG == 1
  SDDBGSERIAL.print("X1 = "); SDDBGSERIAL.println(X1);
  SDDBGSERIAL.print("X2 = "); SDDBGSERIAL.println(X2);
  SDDBGSERIAL.print("B5 = "); SDDBGSERIAL.println(B5);
#endif

  // do pressure calcs
  B6 = B5 - 4000;
  X1 = ((int32_t)b2 * ( (B6 * B6)>>12 )) >> 11;
  X2 = ((int32_t)ac2 * B6) >> 11;
  X3 = X1 + X2;
  B3 = ((((int32_t)ac1*4 + X3) << oversampling) + 2) / 4;

#if BMP085_DEBUG == 1
  SDDBGSERIAL.print("B6 = "); SDDBGSERIAL.println(B6);
  SDDBGSERIAL.print("X1 = "); SDDBGSERIAL.println(X1);
  SDDBGSERIAL.print("X2 = "); SDDBGSERIAL.println(X2);
  SDDBGSERIAL.print("B3 = "); SDDBGSERIAL.println(B3);
#endif

  X1 = ((int32_t)ac3 * B6) >> 13;
  X2 = ((int32_t)b1 * ((B6 * B6) >> 12)) >> 16;
  X3 = ((X1 + X2) + 2) >> 2;
  B4 = ((uint32_t)ac4 * (uint32_t)(X3 + 32768)) >> 15;
  B7 = ((uint32_t)UP - B3) * (uint32_t)( 50000UL >> oversampling );

#if BMP085_DEBUG == 1
  SDDBGSERIAL.print("X1 = "); SDDBGSERIAL.println(X1);
  SDDBGSERIAL.print("X2 = "); SDDBGSERIAL.println(X2);
  SDDBGSERIAL.print("B4 = "); SDDBGSERIAL.println(B4);
  SDDBGSERIAL.print("B7 = "); SDDBGSERIAL.println(B7);
#endif

  if (B7 < 0x80000000) {
    p = (B7 * 2) / B4;
  } else {
    p = (B7 / B4) * 2;
  }
  X1 = (p >> 8) * (p >> 8);
  X1 = (X1 * 3038) >> 16;
  X2 = (-7357 * p) >> 16;

#if BMP085_DEBUG == 1
  SDDBGSERIAL.print("p = "); SDDBGSERIAL.println(p);
  SDDBGSERIAL.print("X1 = "); SDDBGSERIAL.println(X1);
  SDDBGSERIAL.print("X2 = "); SDDBGSERIAL.println(X2);
#endif

  p = p + ((X1 + X2 + (int32_t)3791)>>4);
#if BMP085_DEBUG == 1
  SDDBGSERIAL.print("p = "); SDDBGSERIAL.println(p);
#endif
  return p;
}

 int SensorDriverBmp085::setup(const char* driver, const int address, const int node, const char* type
             #if defined (RADIORF24)
			       , char* mainbuf, size_t lenbuf, RF24Network* network
               #if defined (AES)
			       , uint8_t key[] , uint8_t iv[]
               #endif 
             #endif
			       )
			       {

  SensorDriver::setup(driver,address,node,type
             #if defined (RADIORF24)
		      , mainbuf, lenbuf, network
               #if defined (AES)
		      , key,iv
               #endif
             #endif
		      );

  uint8_t mode= BMP085_HIGHRES;

  if (mode > BMP085_ULTRAHIGHRES) 
    mode = BMP085_ULTRAHIGHRES;
  oversampling = mode;

  if (read8(0xD0) != 0x55) return false;

  /* read calibration data */
  ac1 = read16(BMP085_CAL_AC1);
  ac2 = read16(BMP085_CAL_AC2);
  ac3 = read16(BMP085_CAL_AC3);
  ac4 = read16(BMP085_CAL_AC4);
  ac5 = read16(BMP085_CAL_AC5);
  ac6 = read16(BMP085_CAL_AC6);

  b1 = read16(BMP085_CAL_B1);
  b2 = read16(BMP085_CAL_B2);

  mb = read16(BMP085_CAL_MB);
  mc = read16(BMP085_CAL_MC);
  md = read16(BMP085_CAL_MD);

#if BMP085_DEBUG == 1
  SDDBGSERIAL.print("ac1 = "); SDDBGSERIAL.println(ac1, DEC);
  SDDBGSERIAL.print("ac2 = "); SDDBGSERIAL.println(ac2, DEC);
  SDDBGSERIAL.print("ac3 = "); SDDBGSERIAL.println(ac3, DEC);
  SDDBGSERIAL.print("ac4 = "); SDDBGSERIAL.println(ac4, DEC);
  SDDBGSERIAL.print("ac5 = "); SDDBGSERIAL.println(ac5, DEC);
  SDDBGSERIAL.print("ac6 = "); SDDBGSERIAL.println(ac6, DEC);

  SDDBGSERIAL.print("b1 = "); SDDBGSERIAL.println(b1, DEC);
  SDDBGSERIAL.print("b2 = "); SDDBGSERIAL.println(b2, DEC);

  SDDBGSERIAL.print("mb = "); SDDBGSERIAL.println(mb, DEC);
  SDDBGSERIAL.print("mc = "); SDDBGSERIAL.println(mc, DEC);
  SDDBGSERIAL.print("md = "); SDDBGSERIAL.println(md, DEC);
#endif

  return SD_SUCCESS;

}

int SensorDriverBmp085::prepare(unsigned long& waittime)
{

  _timing=millis();
  waittime= 10ul;

  return SD_SUCCESS;

}

int SensorDriverBmp085::get(long values[],size_t lenvalues)
{

  if (millis() - _timing > MAXDELAYFORREAD)     return SD_INTERNAL_ERROR;

  if (lenvalues>=1) values[0] = (long) round(float(readPressure())/10.);
  //if (lenvalues>=2) values[1] = 0;

  _timing=0;

  return SD_SUCCESS;

}

#if defined(USEAJSON)
aJsonObject* SensorDriverBmp085::getJson()
{
  long values[2];
  aJsonObject* jsonvalues;
  jsonvalues = aJson.createObject();
  //if (SensorDriverBmp085::get2(&pressure,&temperature) == SD_SUCCESS){
  if (SensorDriverBmp085::get(values,2) == SD_SUCCESS){
    // pressure
    aJson.addNumberToObject(jsonvalues, "B10004", values[0]);      
    // if you have a second value add here
    // temperature
    //aJson.addNumberToObject(jsonvalues, "B12101", values[1]);      

  }else{
    aJson.addNullToObject(jsonvalues, "B10004");
    // if you have a second value add here
    //aJson.addNullToObject(jsonvalues, "B12101");
  }
  return jsonvalues;
}
#endif
#endif


#if defined (DAVISWIND1)
int SensorDriverDw1::setup(const char* driver, const int address, const int node, const char* type
  #if defined (RADIORF24)
			   , char* mainbuf, size_t lenbuf, RF24Network* network
    #if defined (AES)
			   , uint8_t key[] , uint8_t iv[]
    #endif 
  #endif 
			   )
{

  SensorDriver::setup(driver,address,node,type
  #if defined (RADIORF24)
		      , mainbuf, lenbuf, network
    #if defined (AES)
		      , key,iv
    #endif
  #endif
		      );
  bool oneshot=true;
  Wire.beginTransmission(_address);
  Wire.write(I2C_WIND_ONESHOT);
  Wire.write(oneshot);
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  delay(10);

  Wire.beginTransmission(_address);
  Wire.write(I2C_WIND_COMMAND);
  Wire.write(I2C_WIND_COMMAND_ONESHOT_STOP);
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  return SD_SUCCESS;

}

int SensorDriverDw1::prepare(unsigned long& waittime)
{

  Wire.beginTransmission(_address);
  Wire.write(I2C_WIND_COMMAND);
  Wire.write(I2C_WIND_COMMAND_ONESHOT_START);
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  _timing=millis();
  // should be better to use SAMPLERATE in config file of i2c-wind
  waittime= 3000ul;

  return SD_SUCCESS;
}

int SensorDriverDw1::get(long values[],size_t lenvalues)
{
  unsigned char msb, lsb;
  if (millis() - _timing > MAXDELAYFORREAD)     return SD_INTERNAL_ERROR;

  // command STOP
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_WIND_COMMAND);
  Wire.write(I2C_WIND_COMMAND_ONESHOT_STOP);
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  delay(10);

  // get DD
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_WIND_DD);
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  delay(10);

  Wire.requestFrom(_address, 2);
  if (Wire.available()<2){
    return SD_INTERNAL_ERROR;
  }
  msb = Wire.read();
  lsb = Wire.read();
  
  if (lenvalues >= 1)  values[0] = (int) lsb<<8 | msb ;

  // get FF
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_WIND_FF);
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  delay(10);
  Wire.requestFrom(_address, 2);
  if (Wire.available()<2){
    return SD_INTERNAL_ERROR;
  }
  msb = Wire.read();
  lsb = Wire.read();
  
  if (lenvalues >= 2)  values[1] = (int) lsb<<8 | msb ;

  // clean register to avoid to get old data next time
  Wire.beginTransmission(_address);
  Wire.write(I2C_WIND_COMMAND);
  Wire.write(I2C_WIND_COMMAND_ONESHOT_STOP);
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  _timing=0;

  return SD_SUCCESS;

}

  #if defined(USEAJSON)
aJsonObject* SensorDriverDw1::getJson()
{
  long values[2];

  aJsonObject* jsonvalues;
  jsonvalues = aJson.createObject();
  if (SensorDriverDw1::get(values,2) == SD_SUCCESS){
    if (values[0] >= 0){
      aJson.addNumberToObject(jsonvalues, "B11001", values[0]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B11001");
    }
    // if you have a second value add here
    if (values[1] >= 0){
      aJson.addNumberToObject(jsonvalues, "B11002", values[1]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B11002");
    }
  }else{
    aJson.addNullToObject(jsonvalues, "B11001");
    // if you have a second value add here
    aJson.addNullToObject(jsonvalues, "B11002");
  }
  return jsonvalues;
}
  #endif
#endif


#if defined (TIPPINGBUCKETRAINGAUGE)
int SensorDriverTbr::setup(const char* driver, const int address, const int node, const char* type
  #if defined (RADIORF24)
			   , char* mainbuf, size_t lenbuf, RF24Network* network
    #if defined (AES)
			   , uint8_t key[] , uint8_t iv[]
    #endif
  #endif
			   )
{

  SensorDriver::setup(driver,address,node,type
  #if defined (RADIORF24)
		      , mainbuf, lenbuf, network
    #if defined (AES)
		      , key,iv
    #endif
  #endif
		      );

  bool oneshot=true;
  Wire.beginTransmission(_address);
  Wire.write(I2C_RAIN_ONESHOT);
  Wire.write(oneshot);
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  delay(10);

  Wire.beginTransmission(_address);
  Wire.write(I2C_RAIN_COMMAND);
  Wire.write(I2C_RAIN_COMMAND_START);
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  return SD_SUCCESS;

}

int SensorDriverTbr::prepare(unsigned long& waittime)
{

  Wire.beginTransmission(_address);
  Wire.write(I2C_RAIN_COMMAND);
  Wire.write(I2C_RAIN_COMMAND_STARTSTOP);
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  _timing=millis();
  waittime= 1ul;

  return SD_SUCCESS;
}

int SensorDriverTbr::get(long values[],size_t lenvalues)
{
  unsigned char msb, lsb;
  //if (millis() - _timing > MAXDELAYFORREAD)     return SD_INTERNAL_ERROR;

  // get tips
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_RAIN_TIPS);
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  delay(10);

  Wire.requestFrom(_address, 2);
  if (Wire.available()<2){
    return SD_INTERNAL_ERROR;
  }
  msb = Wire.read();
  lsb = Wire.read();
  
  if (lenvalues >= 1)  values[0] = ((int) lsb<<8 | msb) * RAINFORTIP ;

  _timing=0;

  return SD_SUCCESS;

}

#if defined(USEAJSON)
aJsonObject* SensorDriverTbr::getJson()
{
  long values[1];

  aJsonObject* jsonvalues;
  jsonvalues = aJson.createObject();
  if (SensorDriverTbr::get(values,1) == SD_SUCCESS){
    if (values[0] >= 0){
      aJson.addNumberToObject(jsonvalues, "B13011", values[0]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B13011");
    }
  }else{
    aJson.addNullToObject(jsonvalues, "B13011");
  }
  return jsonvalues;
}
#endif
#endif


#if defined (TEMPERATUREHUMIDITY_ONESHOT)
int SensorDriverTHoneshot::setup(const char* driver, const int address, const int node, const char* type
  #if defined (RADIORF24)
			   , char* mainbuf, size_t lenbuf, RF24Network* network
    #if defined (AES)
			   , uint8_t key[] , uint8_t iv[]
    #endif
  #endif
			   )
{

  SensorDriver::setup(driver,address,node,type
  #if defined (RADIORF24)
		      , mainbuf, lenbuf, network
    #if defined (AES)
		      , key,iv
    #endif
  #endif
		      );

  bool oneshot=true;
  Wire.beginTransmission(_address);
  Wire.write(I2C_TH_ONESHOT);
  Wire.write(oneshot);
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  delay(10);

  Wire.beginTransmission(_address);
  Wire.write(I2C_TH_COMMAND);
  Wire.write(I2C_TH_COMMAND_ONESHOT_STOP);
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  return SD_SUCCESS;

}

int SensorDriverTHoneshot::prepare(unsigned long& waittime)
{

  Wire.beginTransmission(_address);
  Wire.write(I2C_TH_COMMAND);
  Wire.write(I2C_TH_COMMAND_ONESHOT_START);
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  _timing=millis();
  waittime= 500ul;

  return SD_SUCCESS;
}

int SensorDriverTHoneshot::get(long values[],size_t lenvalues)
{
  unsigned char msb, lsb;
  if (millis() - _timing > MAXDELAYFORREAD)     return SD_INTERNAL_ERROR;

  // command STOP
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_TH_COMMAND);
  Wire.write(I2C_TH_COMMAND_ONESHOT_STOP);
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  delay(10);

  // get temperature
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_TEMPERATURE_SAMPLE);
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  delay(10);

  Wire.requestFrom(_address, 2);
  if (Wire.available()<2){
    return SD_INTERNAL_ERROR;
  }
  msb = Wire.read();
  lsb = Wire.read();
  
  if (lenvalues >= 1) {
    values[0] = ((int) lsb<<8 | msb) ;
    //if (values[0] == 0 ) return SD_INTERNAL_ERROR;
  }

  // get humidity
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_HUMIDITY_SAMPLE);
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  delay(10);

  Wire.requestFrom(_address, 2);
  if (Wire.available()<2){
    return SD_INTERNAL_ERROR;
  }
  msb = Wire.read();
  lsb = Wire.read();
  
  if (lenvalues >= 2) {
    values[1] = ((int) lsb<<8 | msb) ;
    //if (values[1] == 0 ) return SD_INTERNAL_ERROR;
  }

  _timing=0;

  return SD_SUCCESS;

}

#if defined(USEAJSON)
aJsonObject* SensorDriverTHoneshot::getJson()
{
  long values[2];

  aJsonObject* jsonvalues;
  jsonvalues = aJson.createObject();
  if (SensorDriverTHoneshot::get(values,2) == SD_SUCCESS){
    if (values[0] >= 0){
      aJson.addNumberToObject(jsonvalues, "B12101", values[0]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B12101");
    }

    if (values[1] >= 0){
      aJson.addNumberToObject(jsonvalues, "B13003", values[1]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B13003");
    }

  }else{
    aJson.addNullToObject(jsonvalues, "B12101");
    aJson.addNullToObject(jsonvalues, "B13003");
  }
  return jsonvalues;
}
#endif
#endif

#if defined (TEMPERATUREHUMIDITY_REPORT)


int SensorDriverTH60mean::setup(const char* driver, const int address, const int node, const char* type
  #if defined (RADIORF24)
			   , char* mainbuf, size_t lenbuf, RF24Network* network
    #if defined (AES)
			   , uint8_t key[] , uint8_t iv[]
    #endif
  #endif
			   )
{

  SensorDriver::setup(driver,address,node,type
  #if defined (RADIORF24)
		      , mainbuf, lenbuf, network
    #if defined (AES)
		      , key,iv
    #endif
  #endif
		      );

  bool oneshot=false;
  Wire.beginTransmission(_address);
  Wire.write(I2C_TH_ONESHOT);
  Wire.write(oneshot);
 
  short unsigned int ntry=NTRY;
  while  (ntry>0 && Wire.endTransmission() != 0){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#TH60mean setup retry ")));
    ntry--;
    delay(1000);
    Wire.beginTransmission(_address);
    Wire.write(I2C_TH_ONESHOT);
    Wire.write(oneshot);
  }
  if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  // command START
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_TH_COMMAND);
  Wire.write(I2C_TH_COMMAND_START);
  ntry=NTRY;
  while  (ntry>0 && Wire.endTransmission() != 0){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#TH60mean setup retry ")));
    ntry--;
    delay(1000);
    // command START
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_TH_COMMAND);
    Wire.write(I2C_TH_COMMAND_START);
  }
  if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  return SD_SUCCESS;

}

int SensorDriverTH60mean::prepare(unsigned long& waittime)
{

  if (THcounter < 0) THcounter=0;
  THcounter++;
  _timing=millis();

  if (THcounter == 1) {
    // This driver should be the fist of the TH serie; we need to send COMMAND_STOP one time only !
    // command STOP
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_TH_COMMAND);
    Wire.write(I2C_TH_COMMAND_STOP_START);
    short unsigned int ntry=NTRY;
    while  (ntry>0 && Wire.endTransmission() != 0){
      IF_SDSDEBUG(SDDBGSERIAL.println(F("#TH60mean prepare retry ")));
      ntry--;
      delay(1000);
      Wire.beginTransmission(_address);   // Open I2C line in write mode
      Wire.write(I2C_TH_COMMAND);
      Wire.write(I2C_TH_COMMAND_STOP_START);
    }
    if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
    waittime= 100ul;
  }else{
    waittime= 1ul;
  }


  return SD_SUCCESS;
}

int SensorDriverTH60mean::get(long values[],size_t lenvalues)
{
  THcounter--;

  unsigned char msb, lsb;
  if (millis() - _timing > MAXDELAYFORREAD)     return SD_INTERNAL_ERROR;

  // get temperature
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_TEMPERATURE_MEAN60);

  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  delay(10);
  Wire.requestFrom(_address, 2);

  //IF_SDSDEBUG(SDDBGSERIAL.print(F("#available: ")));
  //IF_SDSDEBUG(SDDBGSERIAL.println(Wire.available()));

  if (Wire.available()<2)return SD_INTERNAL_ERROR;

  msb = Wire.read();
  lsb = Wire.read();
  
  if (lenvalues >= 1) {
    values[0] = ((int) lsb<<8 | msb) ;
    //if (values[0] == 0 ) return SD_INTERNAL_ERROR;
  }


  // get humidity
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_HUMIDITY_MEAN60);

  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  delay(10);
  Wire.requestFrom(_address, 2);

  //IF_SDSDEBUG(SDDBGSERIAL.print(F("#available: ")));
  //IF_SDSDEBUG(SDDBGSERIAL.println(Wire.available()));

  if (Wire.available()<2)return SD_INTERNAL_ERROR;

  msb = Wire.read();
  lsb = Wire.read();
  
  if (lenvalues >= 2) {
    values[1] = ((int) lsb<<8 | msb) ;
    //if (values[1] == 0 ) return SD_INTERNAL_ERROR;
  }

  /*
  if (THcounter == 0) {
    // This driver should be the last of the TH serie; we need to send COMMAND_START one time only !
    // command START
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_TH_COMMAND);
    Wire.write(I2C_TH_COMMAND_START);
    short unsigned int ntry=NTRY;
    while  (ntry>0 && Wire.endTransmission() != 0){
      IF_SDSDEBUG(SDDBGSERIAL.println(F("#TH60mean setup retry ")));
      ntry--;
      delay(1000);
      // command START
      Wire.beginTransmission(_address);   // Open I2C line in write mode
      Wire.write(I2C_TH_COMMAND);
      Wire.write(I2C_TH_COMMAND_START);
    }
    if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  }
  */

  _timing=0;

  return SD_SUCCESS;

}

#if defined(USEAJSON)
aJsonObject* SensorDriverTH60mean::getJson()
{
  long values[2];

  aJsonObject* jsonvalues;
  jsonvalues = aJson.createObject();

  short unsigned int ntry=NTRY;

  while (ntry > 0 && SensorDriverTH60mean::get(values,2) != SD_SUCCESS){
    delay(1000);
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#TH60mean get retry ")));
    ntry--;
  }

  if (ntry > 0){
    if (values[0] >= 0){
      aJson.addNumberToObject(jsonvalues, "B12101", values[0]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B12101");
    }
    if (values[1] >= 0){
      aJson.addNumberToObject(jsonvalues, "B13003", values[1]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B13003");
    }

  }else{
    aJson.addNullToObject(jsonvalues, "B12101");
    aJson.addNullToObject(jsonvalues, "B13003");
  }
  return jsonvalues;
}
#endif


int SensorDriverTHmean::setup(const char* driver, const int address, const int node, const char* type
  #if defined (RADIORF24)
			   , char* mainbuf, size_t lenbuf, RF24Network* network
    #if defined (AES)
			   , uint8_t key[] , uint8_t iv[]
    #endif
  #endif
			   )
{

  SensorDriver::setup(driver,address,node,type
  #if defined (RADIORF24)
		      , mainbuf, lenbuf, network
    #if defined (AES)
		      , key,iv
    #endif
  #endif
		      );

  bool oneshot=false;
  Wire.beginTransmission(_address);
  Wire.write(I2C_TH_ONESHOT);
  Wire.write(oneshot);
  short unsigned int ntry=NTRY;
  while  (ntry>0 && Wire.endTransmission() != 0){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#THmean setup retry ")));
    ntry--;
    delay(1000);
    Wire.beginTransmission(_address);
    Wire.write(I2C_TH_ONESHOT);
    Wire.write(oneshot);
  }
  if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  // command START
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_TH_COMMAND);
  Wire.write(I2C_TH_COMMAND_START);
  ntry=NTRY;
  while  (ntry>0 && Wire.endTransmission() != 0){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#TH60mean setup retry ")));
    ntry--;
    delay(1000);
    // command START
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_TH_COMMAND);
    Wire.write(I2C_TH_COMMAND_START);
  }
  if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  return SD_SUCCESS;

}

int SensorDriverTHmean::prepare(unsigned long& waittime)
{

  if (THcounter < 0) THcounter=0;
  THcounter++;
  _timing=millis();

  if (THcounter == 1) {
    // This driver should be the fist of the TH serie; we need to send COMMAND_STOP one time only !
    // command STOP
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_TH_COMMAND);
    Wire.write(I2C_TH_COMMAND_STOP_START);
    short unsigned int ntry=NTRY;
    while  (ntry>0 && Wire.endTransmission() != 0){
      IF_SDSDEBUG(SDDBGSERIAL.println(F("#TH60mean prepare retry ")));
      ntry--;
      delay(1000);
      Wire.beginTransmission(_address);   // Open I2C line in write mode
      Wire.write(I2C_TH_COMMAND);
      Wire.write(I2C_TH_COMMAND_STOP_START);
    }
    if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
    waittime= 100ul;
  }else{
    waittime= 1ul;
  }

  return SD_SUCCESS;
}

int SensorDriverTHmean::get(long values[],size_t lenvalues)
{
  THcounter--;

  unsigned char msb, lsb;
  if (millis() - _timing > MAXDELAYFORREAD)     return SD_INTERNAL_ERROR;

  // get temperature
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_TEMPERATURE_MEAN);

  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  delay(10);
  Wire.requestFrom(_address, 2);

  //IF_SDSDEBUG(SDDBGSERIAL.print(F("#available: ")));
  //IF_SDSDEBUG(SDDBGSERIAL.println(Wire.available()));

  if (Wire.available()<2)return SD_INTERNAL_ERROR;
  msb = Wire.read();
  lsb = Wire.read();

  if (lenvalues >= 1) {
    values[0] = ((int) lsb<<8 | msb) ;
    //if (values[0] == 0 ) return SD_INTERNAL_ERROR;
  }

  // get humidity
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_HUMIDITY_MEAN);

  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  delay(10);
  Wire.requestFrom(_address, 2);

  //IF_SDSDEBUG(SDDBGSERIAL.print(F("#available: ")));
  //IF_SDSDEBUG(SDDBGSERIAL.println(Wire.available()));

   if (Wire.available()<2)return SD_INTERNAL_ERROR;
  msb = Wire.read();
  lsb = Wire.read();

  if (lenvalues >= 2) {
    values[1] = ((int) lsb<<8 | msb) ;
    //if (values[1] == 0 ) return SD_INTERNAL_ERROR;
  }

  /*
  if (THcounter == 0) {
    // This driver should be the last of the TH serie; we need to send COMMAND_START one time only !
    // command START
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_TH_COMMAND);
    Wire.write(I2C_TH_COMMAND_START);
    short unsigned int ntry=NTRY;
    while  (ntry>0 && Wire.endTransmission() != 0){
      IF_SDSDEBUG(SDDBGSERIAL.println(F("#TH60mean setup retry ")));
      ntry--;
      delay(1000);
      // command START
      Wire.beginTransmission(_address);   // Open I2C line in write mode
      Wire.write(I2C_TH_COMMAND);
      Wire.write(I2C_TH_COMMAND_START);
    }
    if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  }
  */
  _timing=0;

  return SD_SUCCESS;

}

#if defined(USEAJSON)
aJsonObject* SensorDriverTHmean::getJson()
{
  long values[2];

  aJsonObject* jsonvalues;
  jsonvalues = aJson.createObject();

  short unsigned int ntry=NTRY;

  while (ntry > 0 && SensorDriverTHmean::get(values,2) != SD_SUCCESS){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#THmean get retry ")));
    delay(1000);
    ntry--;
  }

  if (ntry > 0){
    if (values[0] >= 0){
      aJson.addNumberToObject(jsonvalues, "B12101", values[0]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B12101");
    }

    if (values[1] >= 0){
      aJson.addNumberToObject(jsonvalues, "B13003", values[1]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B13003");      
    }

  }else{
    aJson.addNullToObject(jsonvalues, "B12101");
    aJson.addNullToObject(jsonvalues, "B13003");
  }
  return jsonvalues;
}
#endif



int SensorDriverTHmin::setup(const char* driver, const int address, const int node, const char* type
  #if defined (RADIORF24)
			   , char* mainbuf, size_t lenbuf, RF24Network* network
    #if defined (AES)
			   , uint8_t key[] , uint8_t iv[]
    #endif
  #endif
			   )
{

  SensorDriver::setup(driver,address,node,type
  #if defined (RADIORF24)
		      , mainbuf, lenbuf, network
    #if defined (AES)
		      , key,iv
    #endif
  #endif
		      );

  bool oneshot=false;
  Wire.beginTransmission(_address);
  Wire.write(I2C_TH_ONESHOT);
  Wire.write(oneshot);
 
  short unsigned int ntry=NTRY;
  while  (ntry>0 && Wire.endTransmission() != 0){
    ntry--;
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#THmin setup retry ")));
    delay(1000);
    Wire.beginTransmission(_address);
    Wire.write(I2C_TH_ONESHOT);
    Wire.write(oneshot);
  }
  if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  // command START
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_TH_COMMAND);
  Wire.write(I2C_TH_COMMAND_START);
  ntry=NTRY;
  while  (ntry>0 && Wire.endTransmission() != 0){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#TH60mean setup retry ")));
    ntry--;
    delay(1000);
    // command START
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_TH_COMMAND);
    Wire.write(I2C_TH_COMMAND_START);
  }
  if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  return SD_SUCCESS;

}

int SensorDriverTHmin::prepare(unsigned long& waittime)
{

  if (THcounter < 0) THcounter=0;
  THcounter++;
  _timing=millis();

  if (THcounter == 1) {
    // This driver should be the fist of the TH serie; we need to send COMMAND_STOP one time only !
    // command STOP
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_TH_COMMAND);
    Wire.write(I2C_TH_COMMAND_STOP_START);
    short unsigned int ntry=NTRY;
    while  (ntry>0 && Wire.endTransmission() != 0){
      IF_SDSDEBUG(SDDBGSERIAL.println(F("#TH60mean prepare retry ")));
      ntry--;
      delay(1000);
      Wire.beginTransmission(_address);   // Open I2C line in write mode
      Wire.write(I2C_TH_COMMAND);
      Wire.write(I2C_TH_COMMAND_STOP_START);
    }
    if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
    waittime= 100ul;
  }else{
    waittime= 1ul;
  }

  return SD_SUCCESS;
}

int SensorDriverTHmin::get(long values[],size_t lenvalues)
{
  THcounter--;
  unsigned char msb, lsb;
  if (millis() - _timing > MAXDELAYFORREAD)     return SD_INTERNAL_ERROR;

  // get temperature
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_TEMPERATURE_MIN);

  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  delay(10);
  Wire.requestFrom(_address, 2);

  //IF_SDSDEBUG(SDDBGSERIAL.print(F("#available: ")));
  //IF_SDSDEBUG(SDDBGSERIAL.println(Wire.available()));

  if (Wire.available()<2)return SD_INTERNAL_ERROR;
  msb = Wire.read();
  lsb = Wire.read();

  if (lenvalues >= 1) {
    values[0] = ((int) lsb<<8 | msb) ;
    //if (values[0] == 0 ) return SD_INTERNAL_ERROR;
  }

  // get humidity
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_HUMIDITY_MIN);

  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  delay(10);
  Wire.requestFrom(_address, 2);

  //IF_SDSDEBUG(SDDBGSERIAL.print(F("#available: ")));
  //IF_SDSDEBUG(SDDBGSERIAL.println(Wire.available()));

  if (Wire.available()<2)return SD_INTERNAL_ERROR;
  msb = Wire.read();
  lsb = Wire.read();
  
  if (lenvalues >= 2) {
    values[1] = ((int) lsb<<8 | msb) ;
    //if (values[1] == 0 ) return SD_INTERNAL_ERROR;
  }

  /*
  if (THcounter == 0) {
    // This driver should be the last of the TH serie; we need to send COMMAND_START one time only !
    // command START
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_TH_COMMAND);
    Wire.write(I2C_TH_COMMAND_START);
    short unsigned int ntry=NTRY;
    while  (ntry>0 && Wire.endTransmission() != 0){
      IF_SDSDEBUG(SDDBGSERIAL.println(F("#TH60mean setup retry ")));
      ntry--;
      delay(1000);
      // command START
      Wire.beginTransmission(_address);   // Open I2C line in write mode
      Wire.write(I2C_TH_COMMAND);
      Wire.write(I2C_TH_COMMAND_START);
    }
    if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  }
  */

  _timing=0;

  return SD_SUCCESS;

}

#if defined(USEAJSON)
aJsonObject* SensorDriverTHmin::getJson()
{
  long values[2];

  aJsonObject* jsonvalues;
  jsonvalues = aJson.createObject();

  short unsigned int ntry=NTRY;

  while (ntry > 0 && SensorDriverTHmin::get(values,2) != SD_SUCCESS){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#THmin get retry ")));
    delay(1000);
    ntry--;
  }

  if (ntry > 0){
    if (values[0] >= 0){
      aJson.addNumberToObject(jsonvalues, "B12101", values[0]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B12101");
    }
    if (values[1] >= 0){
      aJson.addNumberToObject(jsonvalues, "B13003", values[1]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B13003");
    }

  }else{
    aJson.addNullToObject(jsonvalues, "B12101");
    aJson.addNullToObject(jsonvalues, "B13003");
  }
  return jsonvalues;
}
#endif

int SensorDriverTHmax::setup(const char* driver, const int address, const int node, const char* type
  #if defined (RADIORF24)
			   , char* mainbuf, size_t lenbuf, RF24Network* network
    #if defined (AES)
			   , uint8_t key[] , uint8_t iv[]
    #endif
  #endif
			   )
{

  SensorDriver::setup(driver,address,node,type
  #if defined (RADIORF24)
		      , mainbuf, lenbuf, network
    #if defined (AES)
		      , key,iv
    #endif
  #endif
		      );

  bool oneshot=false;
  Wire.beginTransmission(_address);
  Wire.write(I2C_TH_ONESHOT);
  Wire.write(oneshot);
  short unsigned int ntry=NTRY;
  while  (ntry>0 && Wire.endTransmission() != 0){
    ntry--;
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#THmax setup retry ")));
    delay(1000);
    Wire.beginTransmission(_address);
    Wire.write(I2C_TH_ONESHOT);
    Wire.write(oneshot);
  }
  if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  // command START
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_TH_COMMAND);
  Wire.write(I2C_TH_COMMAND_START);
  ntry=NTRY;
  while  (ntry>0 && Wire.endTransmission() != 0){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#TH60mean setup retry ")));
    ntry--;
    delay(1000);
    // command START
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_TH_COMMAND);
    Wire.write(I2C_TH_COMMAND_START);
  }
  if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  return SD_SUCCESS;

}

int SensorDriverTHmax::prepare(unsigned long& waittime)
{

  if (THcounter < 0) THcounter=0;
  THcounter++;
  _timing=millis();
  if (THcounter == 1) {
    // This driver should be the fist of the TH serie; we need to send COMMAND_STOP one time only !
    // command STOP
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_TH_COMMAND);
    Wire.write(I2C_TH_COMMAND_STOP_START);
    short unsigned int ntry=NTRY;
    while  (ntry>0 && Wire.endTransmission() != 0){
      IF_SDSDEBUG(SDDBGSERIAL.println(F("#TH60mean prepare retry ")));
      ntry--;
      delay(1000);
      Wire.beginTransmission(_address);   // Open I2C line in write mode
      Wire.write(I2C_TH_COMMAND);
      Wire.write(I2C_TH_COMMAND_STOP_START);
    }
    if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
    waittime= 100ul;
  }else{
    waittime= 1ul;
  }

  return SD_SUCCESS;
}

int SensorDriverTHmax::get(long values[],size_t lenvalues)
{
  THcounter--;
  unsigned char msb, lsb;
  if (millis() - _timing > MAXDELAYFORREAD)     return SD_INTERNAL_ERROR;

  // get temperature
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_TEMPERATURE_MAX);

  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  delay(10);
  Wire.requestFrom(_address, 2);

  //IF_SDSDEBUG(SDDBGSERIAL.print(F("#available: ")));
  //IF_SDSDEBUG(SDDBGSERIAL.println(Wire.available()));

  if (Wire.available()<2)return SD_INTERNAL_ERROR;
  msb = Wire.read();
  lsb = Wire.read();

  if (lenvalues >= 1) {
    values[0] = ((int) lsb<<8 | msb) ;
    //if (values[0] == 0 ) return SD_INTERNAL_ERROR;
  }

  // get humidity
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_HUMIDITY_MAX);

  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  delay(10);
  Wire.requestFrom(_address, 2);

  //IF_SDSDEBUG(SDDBGSERIAL.print(F("#available: ")));
  //IF_SDSDEBUG(SDDBGSERIAL.println(Wire.available()));

  if (Wire.available()<2)return SD_INTERNAL_ERROR;
  msb = Wire.read();
  lsb = Wire.read();

  if (lenvalues >= 2) {
    values[1] = ((int) lsb<<8 | msb) ;
    //if (values[1] == 0 ) return SD_INTERNAL_ERROR;
  }

  /*
  if (THcounter == 0) {
    // This driver should be the last of the TH serie; we need to send COMMAND_START one time only !
    // command START
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_TH_COMMAND);
    Wire.write(I2C_TH_COMMAND_START);
    short unsigned int ntry=NTRY;
    while  (ntry>0 && Wire.endTransmission() != 0){
      IF_SDSDEBUG(SDDBGSERIAL.println(F("#TH60mean setup retry ")));
      ntry--;
      delay(1000);
      // command START
      Wire.beginTransmission(_address);   // Open I2C line in write mode
      Wire.write(I2C_TH_COMMAND);
      Wire.write(I2C_TH_COMMAND_START);
    }
    if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  }
  */

  _timing=0;

  return SD_SUCCESS;

}

#if defined(USEAJSON)
aJsonObject* SensorDriverTHmax::getJson()
{
  long values[2];

  aJsonObject* jsonvalues;
  jsonvalues = aJson.createObject();

  short unsigned int ntry=NTRY;

  while (ntry > 0 && SensorDriverTHmax::get(values,2) != SD_SUCCESS){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#THmax get retry ")));
    delay(1000);
    ntry--;
  }

  if (ntry > 0){
    if (values[0] >= 0){
      aJson.addNumberToObject(jsonvalues, "B12101", values[0]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B12101");
    }

    if (values[1] >= 0){
      aJson.addNumberToObject(jsonvalues, "B13003", values[1]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B13003");
    }

  }else{
    aJson.addNullToObject(jsonvalues, "B12101");
    aJson.addNullToObject(jsonvalues, "B13003");
  }
  return jsonvalues;
}
#endif

#endif


#if defined (SDS011_ONESHOT)
int SensorDriverSDS011oneshot::setup(const char* driver, const int address, const int node, const char* type
  #if defined (RADIORF24)
			   , char* mainbuf, size_t lenbuf, RF24Network* network
    #if defined (AES)
			   , uint8_t key[] , uint8_t iv[]
    #endif
  #endif
			   )
{

  SensorDriver::setup(driver,address,node,type
  #if defined (RADIORF24)
		      , mainbuf, lenbuf, network
    #if defined (AES)
		      , key,iv
    #endif
  #endif
		      );

  bool oneshot=true;
  Wire.beginTransmission(_address);
  Wire.write(I2C_SDS011_ONESHOT);
  Wire.write(oneshot);
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  delay(10);

  Wire.beginTransmission(_address);
  Wire.write(I2C_SDS011_COMMAND);
  Wire.write(I2C_SDS011_COMMAND_ONESHOT_STOP);
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  return SD_SUCCESS;

}

int SensorDriverSDS011oneshot::prepare(unsigned long& waittime)
{

  Wire.beginTransmission(_address);
  Wire.write(I2C_SDS011_COMMAND);
  Wire.write(I2C_SDS011_COMMAND_ONESHOT_START);
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  _timing=millis();
  waittime= 3500ul;

  return SD_SUCCESS;
}

int SensorDriverSDS011oneshot::get(long values[],size_t lenvalues)
{
  unsigned char msb, lsb;
  if (millis() - _timing > MAXDELAYFORREAD)     return SD_INTERNAL_ERROR;

  // command STOP
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_SDS011_COMMAND);
  Wire.write(I2C_SDS011_COMMAND_ONESHOT_STOP);
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  delay(10);

  // get pm25
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_SDS011_PM25);
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  delay(10);

  Wire.requestFrom(_address, 2);
  if (Wire.available()<2){
    return SD_INTERNAL_ERROR;
  }
  msb = Wire.read();
  lsb = Wire.read();
  
  if (lenvalues >= 1) {
    values[0] = ((int) lsb<<8 | msb) ;
    //if (values[0] == 0 ) return SD_INTERNAL_ERROR;
  }

  // get pm10
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_SDS011_PM10);
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  delay(10);

  Wire.requestFrom(_address, 2);
  if (Wire.available()<2){
    return SD_INTERNAL_ERROR;
  }
  msb = Wire.read();
  lsb = Wire.read();
  
  if (lenvalues >= 2) {
    values[1] = ((int) lsb<<8 | msb) ;
    //if (values[1] == 0 ) return SD_INTERNAL_ERROR;
  }

  _timing=0;

  return SD_SUCCESS;

}

#if defined(USEAJSON)
aJsonObject* SensorDriverSDS011oneshot::getJson()
{
  long values[2];

  aJsonObject* jsonvalues;
  jsonvalues = aJson.createObject();
  if (SensorDriverSDS011oneshot::get(values,2) == SD_SUCCESS){
    if (values[0] >= 0){
      aJson.addNumberToObject(jsonvalues, "B15198", values[0]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B15198");
    }

    if (values[1] >= 0){
      aJson.addNumberToObject(jsonvalues, "B15195", values[1]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B15195");
    }

  }else{
    aJson.addNullToObject(jsonvalues, "B15198");
    aJson.addNullToObject(jsonvalues, "B15195");
  }
  return jsonvalues;
}
#endif
#endif

#if defined (SDS011_REPORT)


int SensorDriverSDS01160mean::setup(const char* driver, const int address, const int node, const char* type
  #if defined (RADIORF24)
			   , char* mainbuf, size_t lenbuf, RF24Network* network
    #if defined (AES)
			   , uint8_t key[] , uint8_t iv[]
    #endif
  #endif
			   )
{

  SensorDriver::setup(driver,address,node,type
  #if defined (RADIORF24)
		      , mainbuf, lenbuf, network
    #if defined (AES)
		      , key,iv
    #endif
  #endif
		      );

  bool oneshot=false;
  Wire.beginTransmission(_address);
  Wire.write(I2C_SDS011_ONESHOT);
  Wire.write(oneshot);
 
  short unsigned int ntry=NTRY;
  while  (ntry>0 && Wire.endTransmission() != 0){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#SDS01160mean setup retry ")));
    ntry--;
    delay(1000);
    Wire.beginTransmission(_address);
    Wire.write(I2C_SDS011_ONESHOT);
    Wire.write(oneshot);
  }
  if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  // command START
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_SDS011_COMMAND);
  Wire.write(I2C_SDS011_COMMAND_START);
  ntry=NTRY;
  while  (ntry>0 && Wire.endTransmission() != 0){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#SDS01160mean setup retry ")));
    ntry--;
    delay(1000);
    // command START
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_SDS011_COMMAND);
    Wire.write(I2C_SDS011_COMMAND_START);
  }
  if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  return SD_SUCCESS;

}

int SensorDriverSDS01160mean::prepare(unsigned long& waittime)
{

  if (SDS011counter < 0) SDS011counter=0;
  SDS011counter++;
  _timing=millis();

  if (SDS011counter == 1) {
    // This driver should be the fist of the SDS011 serie; we need to send COMMAND_STOP one time only !
    // command STOP
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_SDS011_COMMAND);
    Wire.write(I2C_SDS011_COMMAND_STOP_START);
    short unsigned int ntry=NTRY;
    while  (ntry>0 && Wire.endTransmission() != 0){
      IF_SDSDEBUG(SDDBGSERIAL.println(F("#SDS01160mean prepare retry ")));
      ntry--;
      delay(1000);
      Wire.beginTransmission(_address);   // Open I2C line in write mode
      Wire.write(I2C_SDS011_COMMAND);
      Wire.write(I2C_SDS011_COMMAND_STOP_START);
    }
    if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
    waittime= 100ul;
  }else{
    waittime= 1ul;
  }


  return SD_SUCCESS;
}

int SensorDriverSDS01160mean::get(long values[],size_t lenvalues)
{
  SDS011counter--;

  unsigned char msb, lsb;
  if (millis() - _timing > MAXDELAYFORREAD)     return SD_INTERNAL_ERROR;

  // get temperature
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_TEMPERATURE_MEAN60);

  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  delay(10);
  Wire.requestFrom(_address, 2);

  //IF_SDSDEBUG(SDDBGSERIAL.print(F("#available: ")));
  //IF_SDSDEBUG(SDDBGSERIAL.println(Wire.available()));

  if (Wire.available()<2)return SD_INTERNAL_ERROR;

  msb = Wire.read();
  lsb = Wire.read();
  
  if (lenvalues >= 1) {
    values[0] = ((int) lsb<<8 | msb) ;
    //if (values[0] == 0 ) return SD_INTERNAL_ERROR;
  }


  // get humidity
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_HUMIDITY_MEAN60);

  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  delay(10);
  Wire.requestFrom(_address, 2);

  //IF_SDSDEBUG(SDDBGSERIAL.print(F("#available: ")));
  //IF_SDSDEBUG(SDDBGSERIAL.println(Wire.available()));

  if (Wire.available()<2)return SD_INTERNAL_ERROR;

  msb = Wire.read();
  lsb = Wire.read();
  
  if (lenvalues >= 2) {
    values[1] = ((int) lsb<<8 | msb) ;
    //if (values[1] == 0 ) return SD_INTERNAL_ERROR;
  }

  /*
  if (SDS011counter == 0) {
    // This driver should be the last of the SDS011 serie; we need to send COMMAND_START one time only !
    // command START
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_SDS011_COMMAND);
    Wire.write(I2C_SDS011_COMMAND_START);
    short unsigned int ntry=NTRY;
    while  (ntry>0 && Wire.endTransmission() != 0){
      IF_SDSDEBUG(SDDBGSERIAL.println(F("#SDS01160mean setup retry ")));
      ntry--;
      delay(1000);
      // command START
      Wire.beginTransmission(_address);   // Open I2C line in write mode
      Wire.write(I2C_SDS011_COMMAND);
      Wire.write(I2C_SDS011_COMMAND_START);
    }
    if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  }
  */

  _timing=0;

  return SD_SUCCESS;

}

#if defined(USEAJSON)
aJsonObject* SensorDriverSDS01160mean::getJson()
{
  long values[2];

  aJsonObject* jsonvalues;
  jsonvalues = aJson.createObject();

  short unsigned int ntry=NTRY;

  while (ntry > 0 && SensorDriverSDS01160mean::get(values,2) != SD_SUCCESS){
    delay(1000);
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#SDS01160mean get retry ")));
    ntry--;
  }

  if (ntry > 0){
    if (values[0] >= 0){
      aJson.addNumberToObject(jsonvalues, "B15198", values[0]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B15198");
    }
    if (values[1] >= 0){
      aJson.addNumberToObject(jsonvalues, "B15195", values[1]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B15195");
    }

  }else{
    aJson.addNullToObject(jsonvalues, "B15198");
    aJson.addNullToObject(jsonvalues, "B15195");
  }
  return jsonvalues;
}
#endif


int SensorDriverSDS011mean::setup(const char* driver, const int address, const int node, const char* type
  #if defined (RADIORF24)
			   , char* mainbuf, size_t lenbuf, RF24Network* network
    #if defined (AES)
			   , uint8_t key[] , uint8_t iv[]
    #endif
  #endif
			   )
{

  SensorDriver::setup(driver,address,node,type
  #if defined (RADIORF24)
		      , mainbuf, lenbuf, network
    #if defined (AES)
		      , key,iv
    #endif
  #endif
		      );

  bool oneshot=false;
  Wire.beginTransmission(_address);
  Wire.write(I2C_SDS011_ONESHOT);
  Wire.write(oneshot);
  short unsigned int ntry=NTRY;
  while  (ntry>0 && Wire.endTransmission() != 0){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#SDS011mean setup retry ")));
    ntry--;
    delay(1000);
    Wire.beginTransmission(_address);
    Wire.write(I2C_SDS011_ONESHOT);
    Wire.write(oneshot);
  }
  if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  // command START
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_SDS011_COMMAND);
  Wire.write(I2C_SDS011_COMMAND_START);
  ntry=NTRY;
  while  (ntry>0 && Wire.endTransmission() != 0){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#SDS01160mean setup retry ")));
    ntry--;
    delay(1000);
    // command START
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_SDS011_COMMAND);
    Wire.write(I2C_SDS011_COMMAND_START);
  }
  if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  return SD_SUCCESS;

}

int SensorDriverSDS011mean::prepare(unsigned long& waittime)
{

  if (SDS011counter < 0) SDS011counter=0;
  SDS011counter++;
  _timing=millis();

  if (SDS011counter == 1) {
    // This driver should be the fist of the SDS011 serie; we need to send COMMAND_STOP one time only !
    // command STOP
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_SDS011_COMMAND);
    Wire.write(I2C_SDS011_COMMAND_STOP_START);
    short unsigned int ntry=NTRY;
    while  (ntry>0 && Wire.endTransmission() != 0){
      IF_SDSDEBUG(SDDBGSERIAL.println(F("#SDS01160mean prepare retry ")));
      ntry--;
      delay(1000);
      Wire.beginTransmission(_address);   // Open I2C line in write mode
      Wire.write(I2C_SDS011_COMMAND);
      Wire.write(I2C_SDS011_COMMAND_STOP_START);
    }
    if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
    waittime= 100ul;
  }else{
    waittime= 1ul;
  }

  return SD_SUCCESS;
}

int SensorDriverSDS011mean::get(long values[],size_t lenvalues)
{
  SDS011counter--;

  unsigned char msb, lsb;
  if (millis() - _timing > MAXDELAYFORREAD)     return SD_INTERNAL_ERROR;

  // get temperature
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_TEMPERATURE_MEAN);

  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  delay(10);
  Wire.requestFrom(_address, 2);

  //IF_SDSDEBUG(SDDBGSERIAL.print(F("#available: ")));
  //IF_SDSDEBUG(SDDBGSERIAL.println(Wire.available()));

  if (Wire.available()<2)return SD_INTERNAL_ERROR;
  msb = Wire.read();
  lsb = Wire.read();

  if (lenvalues >= 1) {
    values[0] = ((int) lsb<<8 | msb) ;
    //if (values[0] == 0 ) return SD_INTERNAL_ERROR;
  }

  // get humidity
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_HUMIDITY_MEAN);

  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  delay(10);
  Wire.requestFrom(_address, 2);

  //IF_SDSDEBUG(SDDBGSERIAL.print(F("#available: ")));
  //IF_SDSDEBUG(SDDBGSERIAL.println(Wire.available()));

   if (Wire.available()<2)return SD_INTERNAL_ERROR;
  msb = Wire.read();
  lsb = Wire.read();

  if (lenvalues >= 2) {
    values[1] = ((int) lsb<<8 | msb) ;
    //if (values[1] == 0 ) return SD_INTERNAL_ERROR;
  }

  /*
  if (SDS011counter == 0) {
    // This driver should be the last of the SDS011 serie; we need to send COMMAND_START one time only !
    // command START
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_SDS011_COMMAND);
    Wire.write(I2C_SDS011_COMMAND_START);
    short unsigned int ntry=NTRY;
    while  (ntry>0 && Wire.endTransmission() != 0){
      IF_SDSDEBUG(SDDBGSERIAL.println(F("#SDS01160mean setup retry ")));
      ntry--;
      delay(1000);
      // command START
      Wire.beginTransmission(_address);   // Open I2C line in write mode
      Wire.write(I2C_SDS011_COMMAND);
      Wire.write(I2C_SDS011_COMMAND_START);
    }
    if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  }
  */
  _timing=0;

  return SD_SUCCESS;

}

#if defined(USEAJSON)
aJsonObject* SensorDriverSDS011mean::getJson()
{
  long values[2];

  aJsonObject* jsonvalues;
  jsonvalues = aJson.createObject();

  short unsigned int ntry=NTRY;

  while (ntry > 0 && SensorDriverSDS011mean::get(values,2) != SD_SUCCESS){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#SDS011mean get retry ")));
    delay(1000);
    ntry--;
  }

  if (ntry > 0){
    if (values[0] >= 0){
      aJson.addNumberToObject(jsonvalues, "B15198", values[0]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B15198");
    }

    if (values[1] >= 0){
      aJson.addNumberToObject(jsonvalues, "B15195", values[1]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B15195");      
    }

  }else{
    aJson.addNullToObject(jsonvalues, "B15198");
    aJson.addNullToObject(jsonvalues, "B15195");
  }
  return jsonvalues;
}
#endif



int SensorDriverSDS011min::setup(const char* driver, const int address, const int node, const char* type
  #if defined (RADIORF24)
			   , char* mainbuf, size_t lenbuf, RF24Network* network
    #if defined (AES)
			   , uint8_t key[] , uint8_t iv[]
    #endif
  #endif
			   )
{

  SensorDriver::setup(driver,address,node,type
  #if defined (RADIORF24)
		      , mainbuf, lenbuf, network
    #if defined (AES)
		      , key,iv
    #endif
  #endif
		      );

  bool oneshot=false;
  Wire.beginTransmission(_address);
  Wire.write(I2C_SDS011_ONESHOT);
  Wire.write(oneshot);
 
  short unsigned int ntry=NTRY;
  while  (ntry>0 && Wire.endTransmission() != 0){
    ntry--;
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#SDS011min setup retry ")));
    delay(1000);
    Wire.beginTransmission(_address);
    Wire.write(I2C_SDS011_ONESHOT);
    Wire.write(oneshot);
  }
  if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  // command START
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_SDS011_COMMAND);
  Wire.write(I2C_SDS011_COMMAND_START);
  ntry=NTRY;
  while  (ntry>0 && Wire.endTransmission() != 0){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#SDS01160mean setup retry ")));
    ntry--;
    delay(1000);
    // command START
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_SDS011_COMMAND);
    Wire.write(I2C_SDS011_COMMAND_START);
  }
  if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  return SD_SUCCESS;

}

int SensorDriverSDS011min::prepare(unsigned long& waittime)
{

  if (SDS011counter < 0) SDS011counter=0;
  SDS011counter++;
  _timing=millis();

  if (SDS011counter == 1) {
    // This driver should be the fist of the SDS011 serie; we need to send COMMAND_STOP one time only !
    // command STOP
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_SDS011_COMMAND);
    Wire.write(I2C_SDS011_COMMAND_STOP_START);
    short unsigned int ntry=NTRY;
    while  (ntry>0 && Wire.endTransmission() != 0){
      IF_SDSDEBUG(SDDBGSERIAL.println(F("#SDS01160mean prepare retry ")));
      ntry--;
      delay(1000);
      Wire.beginTransmission(_address);   // Open I2C line in write mode
      Wire.write(I2C_SDS011_COMMAND);
      Wire.write(I2C_SDS011_COMMAND_STOP_START);
    }
    if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
    waittime= 100ul;
  }else{
    waittime= 1ul;
  }

  return SD_SUCCESS;
}

int SensorDriverSDS011min::get(long values[],size_t lenvalues)
{
  SDS011counter--;
  unsigned char msb, lsb;
  if (millis() - _timing > MAXDELAYFORREAD)     return SD_INTERNAL_ERROR;

  // get temperature
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_TEMPERATURE_MIN);

  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  delay(10);
  Wire.requestFrom(_address, 2);

  //IF_SDSDEBUG(SDDBGSERIAL.print(F("#available: ")));
  //IF_SDSDEBUG(SDDBGSERIAL.println(Wire.available()));

  if (Wire.available()<2)return SD_INTERNAL_ERROR;
  msb = Wire.read();
  lsb = Wire.read();

  if (lenvalues >= 1) {
    values[0] = ((int) lsb<<8 | msb) ;
    //if (values[0] == 0 ) return SD_INTERNAL_ERROR;
  }

  // get humidity
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_HUMIDITY_MIN);

  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  delay(10);
  Wire.requestFrom(_address, 2);

  //IF_SDSDEBUG(SDDBGSERIAL.print(F("#available: ")));
  //IF_SDSDEBUG(SDDBGSERIAL.println(Wire.available()));

  if (Wire.available()<2)return SD_INTERNAL_ERROR;
  msb = Wire.read();
  lsb = Wire.read();
  
  if (lenvalues >= 2) {
    values[1] = ((int) lsb<<8 | msb) ;
    //if (values[1] == 0 ) return SD_INTERNAL_ERROR;
  }

  /*
  if (SDS011counter == 0) {
    // This driver should be the last of the SDS011 serie; we need to send COMMAND_START one time only !
    // command START
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_SDS011_COMMAND);
    Wire.write(I2C_SDS011_COMMAND_START);
    short unsigned int ntry=NTRY;
    while  (ntry>0 && Wire.endTransmission() != 0){
      IF_SDSDEBUG(SDDBGSERIAL.println(F("#SDS01160mean setup retry ")));
      ntry--;
      delay(1000);
      // command START
      Wire.beginTransmission(_address);   // Open I2C line in write mode
      Wire.write(I2C_SDS011_COMMAND);
      Wire.write(I2C_SDS011_COMMAND_START);
    }
    if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  }
  */

  _timing=0;

  return SD_SUCCESS;

}

#if defined(USEAJSON)
aJsonObject* SensorDriverSDS011min::getJson()
{
  long values[2];

  aJsonObject* jsonvalues;
  jsonvalues = aJson.createObject();

  short unsigned int ntry=NTRY;

  while (ntry > 0 && SensorDriverSDS011min::get(values,2) != SD_SUCCESS){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#SDS011min get retry ")));
    delay(1000);
    ntry--;
  }

  if (ntry > 0){
    if (values[0] >= 0){
      aJson.addNumberToObject(jsonvalues, "B15198", values[0]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B15198");
    }
    if (values[1] >= 0){
      aJson.addNumberToObject(jsonvalues, "B15195", values[1]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B15195");
    }

  }else{
    aJson.addNullToObject(jsonvalues, "B15198");
    aJson.addNullToObject(jsonvalues, "B15195");
  }
  return jsonvalues;
}
#endif

int SensorDriverSDS011max::setup(const char* driver, const int address, const int node, const char* type
  #if defined (RADIORF24)
			   , char* mainbuf, size_t lenbuf, RF24Network* network
    #if defined (AES)
			   , uint8_t key[] , uint8_t iv[]
    #endif
  #endif
			   )
{

  SensorDriver::setup(driver,address,node,type
  #if defined (RADIORF24)
		      , mainbuf, lenbuf, network
    #if defined (AES)
		      , key,iv
    #endif
  #endif
		      );

  bool oneshot=false;
  Wire.beginTransmission(_address);
  Wire.write(I2C_SDS011_ONESHOT);
  Wire.write(oneshot);
  short unsigned int ntry=NTRY;
  while  (ntry>0 && Wire.endTransmission() != 0){
    ntry--;
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#SDS011max setup retry ")));
    delay(1000);
    Wire.beginTransmission(_address);
    Wire.write(I2C_SDS011_ONESHOT);
    Wire.write(oneshot);
  }
  if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  // command START
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_SDS011_COMMAND);
  Wire.write(I2C_SDS011_COMMAND_START);
  ntry=NTRY;
  while  (ntry>0 && Wire.endTransmission() != 0){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#SDS01160mean setup retry ")));
    ntry--;
    delay(1000);
    // command START
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_SDS011_COMMAND);
    Wire.write(I2C_SDS011_COMMAND_START);
  }
  if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  return SD_SUCCESS;

}

int SensorDriverSDS011max::prepare(unsigned long& waittime)
{

  if (SDS011counter < 0) SDS011counter=0;
  SDS011counter++;
  _timing=millis();
  if (SDS011counter == 1) {
    // This driver should be the fist of the SDS011 serie; we need to send COMMAND_STOP one time only !
    // command STOP
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_SDS011_COMMAND);
    Wire.write(I2C_SDS011_COMMAND_STOP_START);
    short unsigned int ntry=NTRY;
    while  (ntry>0 && Wire.endTransmission() != 0){
      IF_SDSDEBUG(SDDBGSERIAL.println(F("#SDS01160mean prepare retry ")));
      ntry--;
      delay(1000);
      Wire.beginTransmission(_address);   // Open I2C line in write mode
      Wire.write(I2C_SDS011_COMMAND);
      Wire.write(I2C_SDS011_COMMAND_STOP_START);
    }
    if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
    waittime= 100ul;
  }else{
    waittime= 1ul;
  }

  return SD_SUCCESS;
}

int SensorDriverSDS011max::get(long values[],size_t lenvalues)
{
  SDS011counter--;
  unsigned char msb, lsb;
  if (millis() - _timing > MAXDELAYFORREAD)     return SD_INTERNAL_ERROR;

  // get temperature
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_TEMPERATURE_MAX);

  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  delay(10);
  Wire.requestFrom(_address, 2);

  //IF_SDSDEBUG(SDDBGSERIAL.print(F("#available: ")));
  //IF_SDSDEBUG(SDDBGSERIAL.println(Wire.available()));

  if (Wire.available()<2)return SD_INTERNAL_ERROR;
  msb = Wire.read();
  lsb = Wire.read();

  if (lenvalues >= 1) {
    values[0] = ((int) lsb<<8 | msb) ;
    //if (values[0] == 0 ) return SD_INTERNAL_ERROR;
  }

  // get humidity
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_HUMIDITY_MAX);

  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  delay(10);
  Wire.requestFrom(_address, 2);

  //IF_SDSDEBUG(SDDBGSERIAL.print(F("#available: ")));
  //IF_SDSDEBUG(SDDBGSERIAL.println(Wire.available()));

  if (Wire.available()<2)return SD_INTERNAL_ERROR;
  msb = Wire.read();
  lsb = Wire.read();

  if (lenvalues >= 2) {
    values[1] = ((int) lsb<<8 | msb) ;
    //if (values[1] == 0 ) return SD_INTERNAL_ERROR;
  }

  /*
  if (SDS011counter == 0) {
    // This driver should be the last of the SDS011 serie; we need to send COMMAND_START one time only !
    // command START
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_SDS011_COMMAND);
    Wire.write(I2C_SDS011_COMMAND_START);
    short unsigned int ntry=NTRY;
    while  (ntry>0 && Wire.endTransmission() != 0){
      IF_SDSDEBUG(SDDBGSERIAL.println(F("#SDS01160mean setup retry ")));
      ntry--;
      delay(1000);
      // command START
      Wire.beginTransmission(_address);   // Open I2C line in write mode
      Wire.write(I2C_SDS011_COMMAND);
      Wire.write(I2C_SDS011_COMMAND_START);
    }
    if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  }
  */

  _timing=0;

  return SD_SUCCESS;

}

#if defined(USEAJSON)
aJsonObject* SensorDriverSDS011max::getJson()
{
  long values[2];

  aJsonObject* jsonvalues;
  jsonvalues = aJson.createObject();

  short unsigned int ntry=NTRY;

  while (ntry > 0 && SensorDriverSDS011max::get(values,2) != SD_SUCCESS){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#SDS011max get retry ")));
    delay(1000);
    ntry--;
  }

  if (ntry > 0){
    if (values[0] >= 0){
      aJson.addNumberToObject(jsonvalues, "B15198", values[0]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B15198");
    }

    if (values[1] >= 0){
      aJson.addNumberToObject(jsonvalues, "B15195", values[1]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B15195");
    }

  }else{
    aJson.addNullToObject(jsonvalues, "B15198");
    aJson.addNullToObject(jsonvalues, "B15195");
  }
  return jsonvalues;
}
#endif

#endif
