#include "SensorDriverb.h"

//void SensorDriverInit()
//{
//  Wire.begin();                // join i2c bus as master
//}


//  TO BE changed !!!  no global variable
int THcounter=0;
int SDS011counter=0;
bool SDSMICSstarted=false;

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
#if defined (HI7021DRIVER)
      if (strcmp(type, "HI7") == 0)
	return new SensorDriverSI7021();
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
      if (strcmp(type, "TBS") == 0)
	return new SensorDriverTbr();
      else
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
#if defined (MICS4514_ONESHOT)
      if (strcmp(type, "SMI") == 0)
	return new SensorDriverMICS4514oneshot();
      else
#endif
#if defined (MICS4514_REPORT)
      if (strcmp(type, "IMI") == 0)   // istantaneous
	return new SensorDriverMICS451460mean();
      else
      if (strcmp(type, "MMI") == 0)   // mean
	return new SensorDriverMICS4514mean();
      else
      if (strcmp(type, "NMI") == 0)   // min
	return new SensorDriverMICS4514min();
      else
      if (strcmp(type, "XMI") == 0)   //max
	return new SensorDriverMICS4514max();
      else
#endif

#if defined (SCD_ONESHOT)
	if (strcmp(type, "SCD") == 0) {
	  return new SensorDriverSCDoneshot();
	} else
#endif

#if defined (SHTDRIVER)
	if (strcmp(type, "SHT") == 0) {
	  return new SensorDriverSHT85();
        } else
#endif
#if defined (SPS_ONESHOT)
	  if (strcmp(type, "SPS") == 0) {
	    return new SensorDriverSPSoneshot();
	  } 
#endif

      return NULL;
    } else

#if defined (RADIORF24)
    if (strcmp(driver, "RF24") == 0){
      return new SensorDriverRF24();
    } else
#endif

      if (strcmp(driver, "SERI") == 0){
#if defined (SDS011_LOCALSERIAL)
	if (strcmp(type, "SSD") == 0) {
	  return new SensorDriverSDS011oneshotSerial();
	} else 
#endif
#if defined (HPM_ONESHOT)
	  if (strcmp(type, "HPM") == 0) {
	    return new SensorDriverHPMoneshotSerial();
	  } else
#endif
#if defined (PMS_ONESHOT)
	  if (strcmp(type, "PMS") == 0) {
	    return new SensorDriverPMSoneshotSerial();
	  } 
#endif	
    } else
    {
      return NULL;
    }

  return NULL;

}
SensorDriver::~SensorDriver() {}


#if defined (USEGETDATA)
int SensorDriver::getdata(uint32_t& data,uint16_t& width)
{
  data=0xFFFFFFFF;
  width=0xFFFF;
  return SD_INTERNAL_ERROR;
}
#endif


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

int SensorDriverTmp::prepare(uint32_t& waittime)
{

  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write((byte)0x01);            // Set the register pointer to (0x01)
  Wire.write((byte)0xE1);            // Set resolution and SHUTDOWN MODE and one shot
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  _timing=millis();
  waittime= 500ul;

  return SD_SUCCESS;

}

int SensorDriverTmp::get(uint32_t values[],size_t lenvalues)
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
  if (lenvalues >= 1)  values[0] = (uint32_t)(TemperatureSum*6.25 + 27315.) ;

  _timing=0;

  return SD_SUCCESS;

}

#if defined (USEGETDATA)
int SensorDriverTmp::getdata(uint32_t& data,uint16_t& width)
{
  data=0xFFFFFFFF;
  width=0xFFFF;
  return SD_INTERNAL_ERROR;
}
#endif

  #if defined(USEAJSON)
aJsonObject* SensorDriverTmp::getJson()
{
  uint32_t values[1];
  aJsonObject* jsonvalues;
  jsonvalues = aJson.createObject();
  if (SensorDriverTmp::get(values,1) == SD_SUCCESS){
    aJson.addNumberToObject(jsonvalues, "B12101", (int32_t)values[0]);      
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

int SensorDriverAdt7420::prepare(uint32_t& waittime)
{

  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write((byte)0x03);                              // Set the register pointer to (0x01)
  Wire.write((byte)0x20);                              // Set resolution and one shot
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  waittime=250ul;
  _timing=millis();

  return SD_SUCCESS;

}

int SensorDriverAdt7420::get(uint32_t values[],size_t lenvalues)
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

  if (lenvalues >= 1)  values[0] = (uint32_t)(TemperatureSum*6.25 + 27315.) ;
  _timing=0;

  return SD_SUCCESS;

}

#if defined (USEGETDATA)
int SensorDriverAdt7420::getdata(uint32_t &data,uint16_t &width)
{
  /*
    scale: The exponent of the  power of 10 by which the value of the element has been multiplied prior to encoding 
    reference value: A number to be subtracted from the element, after scaling (if any), and prior to encoding 
    data width (bits): The number of bits the element requires for representation in data
  */

  uint32_t values[1];
  width=16;
  const uint32_t reference=22315;
  
  if (SensorDriverAdt7420::get(values,1) == SD_SUCCESS){
    data=(values[0]-reference) ;// << (sizeof(values[1])-width);
  }else{
    data=0xFFFFFFFF;
    width=0xFFFF;
    return SD_INTERNAL_ERROR;
  }
  return SD_SUCCESS;
}
#endif

  #if defined(USEAJSON)
aJsonObject* SensorDriverAdt7420::getJson()
{
  uint32_t values[1];
  aJsonObject* jsonvalues;
  jsonvalues = aJson.createObject();
  
  if (SensorDriverAdt7420::get(values,1) == SD_SUCCESS){
    aJson.addNumberToObject(jsonvalues, "B12101", (int32_t)values[0]);      
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

#if defined(USEARDUINOJSON)
int SensorDriverAdt7420::getJson(char *json_buffer, size_t json_buffer_length)
{
  uint32_t values[1];
  StaticJsonDocument<100> doc;

  if (get(values,1) == SD_SUCCESS){
    if (values[0] >= 0){
      doc["B12101"]= values[0];      
    }else{
      doc["B12101"]=serialized("null");
    }

  }else{
    doc["B12101"]=serialized("null");
  }

  serializeJson(doc,json_buffer, json_buffer_length);
  return SD_SUCCESS;
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

int SensorDriverRF24::prepare(uint32_t& waittime)
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

  uint32_t start_at = millis();

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
  //waittime=(uint32_t)waittimei->valueint;
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
  
  IF_SDSDEBUG(SDDBGSERIAL.println(F("#Radio Sending... getJson")));
  
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
  
  uint32_t start_at = millis();

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

int SensorDriverRF24::get(uint32_t values[], size_t lenvalues)
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

int SensorDriverHih6100::prepare(uint32_t& waittime)
{

  Wire.beginTransmission(_address);   // Open I2C line in write mode
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  _timing=millis();
  waittime= 40ul;   //The measurement cycle duration is typically 36.65 ms for temperature and humidity readings

  return SD_SUCCESS;

}

int SensorDriverHih6100::get(uint32_t values[],size_t lenvalues)
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
  uint32_t h;
  uint32_t t;
  
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

      h = (((uint32_t) (x & 0x3f)) << 8) | y;

      x = Wire.read();
      y = Wire.read();
      //IF_SDSDEBUG(SDDBGSERIAL.print(F("#hih read temperature x,y: ")));
      //IF_SDSDEBUG(SDDBGSERIAL.print(x));
      //IF_SDSDEBUG(SDDBGSERIAL.print(F(" , ")));
      //IF_SDSDEBUG(SDDBGSERIAL.println(y));

      t = (((uint32_t) x) << 6) | ((y & 0xfc) >> 2);
      
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

  if (lenvalues >= 1)  values[0] = (uint32_t) round (float(h) / 16382. * 100.) ;
  if (lenvalues >= 2)  values[1] = (uint32_t) round((float(t) / 16382. * 165. - 40.) * 100. + 27315.) ;
  _timing=0;

  return SD_SUCCESS;

}

#if defined (USEGETDATA)
int SensorDriverHih6100::getdata(uint32_t& data,uint16_t& width)
{
  /*
    scale: The exponent of the  power of 10 by which the value of the element has been multiplied prior to encoding 
    reference value: A number to be subtracted from the element, after scaling (if any), and prior to encoding 
    data width (bits): The number of bits the element requires for representation in data
  */
  
  uint32_t values[1];
  width=7;
  const uint32_t reference=0;
  
  if (SensorDriverHih6100::get(values,1) == SD_SUCCESS){
    data=(values[0]-reference);// << (sizeof(values[1])-width);
  }else{
    data=0xFFFFFFFF;
    width=0xFFFF;
    return SD_INTERNAL_ERROR;
  }
  return SD_SUCCESS;
}
#endif

#if defined(USEAJSON)
aJsonObject* SensorDriverHih6100::getJson()
{
  uint32_t values[2];

  aJsonObject* jsonvalues;
  jsonvalues = aJson.createObject();
  //if (SensorDriverTmp::get2(&humidity,&temperature) == SD_SUCCESS){
  if (SensorDriverHih6100::get(values,2) == SD_SUCCESS){
    aJson.addNumberToObject(jsonvalues, "B13003", (int32_t)values[0]);      

#if defined(SECONDARYPARAMETER)
    // if you have a second value add here
    aJson.addNumberToObject(jsonvalues, "B12101", (int32_t)values[1]);      
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
#if defined(USEARDUINOJSON)
int SensorDriverHih6100::getJson(char *json_buffer, size_t json_buffer_length)
{
  uint32_t values[1];
  StaticJsonDocument<200> doc;

  if (get(values,1) == SD_SUCCESS){
    if (values[0] >= 0){
      doc["B13003"]= values[0];      
    }else{
      doc["B13003"]=serialized("null");
    }
#if defined(SECONDARYPARAMETER)
    // if you have a second value add here
    if (values[1] >= 0){
      doc["B12101"]= values[1];      
    }else{
      doc["B12101"]=serialized("null");
    }
#endif    
  }else{
    doc["B13003"]=serialized("null");
#if defined(SECONDARYPARAMETER)
    // if you have a second value add here
    doc["B12101"]=serialized("null");
#endif
  }

  serializeJson(doc,json_buffer, json_buffer_length);
  return SD_SUCCESS;
}
#endif
#endif

#if defined (HYTDRIVER)
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

int SensorDriverHyt271::prepare(uint32_t &waittime) {
	Wire.beginTransmission(_address);   	 //Open I2C line in write mode
	
	if (Wire.endTransmission())
		return SD_INTERNAL_ERROR;			 //End Write Transmission 
	
	_timing = millis();
	waittime = 100ul;   					 //The measurement cycle duration is typically 100 ms max for temperature and humidity readings
	
	return SD_SUCCESS;
}

int SensorDriverHyt271::get(uint32_t values[], size_t lenvalues) {
	if (millis() - _timing > MAXDELAYFORREAD)
		return SD_INTERNAL_ERROR;
	
	float humidity;
	float temperature;
	
	Wire.requestFrom(_address, I2C_HYT271_READ_HT_DATA_LENGTH);
	
	if (Wire.available() == I2C_HYT271_READ_HT_DATA_LENGTH)
		HYT271_getHT((uint32_t) Wire.read() << 24 | (uint32_t) Wire.read() << 16 | (uint32_t) Wire.read() << 8 | (uint32_t) Wire.read(), &humidity, &temperature);
	else return SD_INTERNAL_ERROR;

	if (lenvalues >= 1)
		values[0] = (uint32_t) round(humidity);
	
	if (lenvalues >= 2)
		values[1] = (uint32_t) round(temperature*100 + 27315);
	
	_timing = 0;
	return SD_SUCCESS;
}

#if defined (USEGETDATA)
int SensorDriverHyt271::getdata(uint32_t& data,uint16_t& width)
{
  data=0xFFFFFFFF;
  width=0xFFFF;
  return SD_INTERNAL_ERROR;
}
#endif

#if defined(USEAJSON)
aJsonObject* SensorDriverHyt271::getJson() {
	uint32_t values[2];
	aJsonObject* jsonvalues;
	jsonvalues = aJson.createObject();
	
	if (SensorDriverHyt271::get(values,2) == SD_SUCCESS) {
		aJson.addNumberToObject(jsonvalues, "B13003", (int32_t)values[0]);
		aJson.addNumberToObject(jsonvalues, "B12101", (int32_t)values[1]);
	}
	else {
		aJson.addNullToObject(jsonvalues, (int32_t)"B12101");
		aJson.addNullToObject(jsonvalues, (int32_t)"B13003");
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
  Wire.write(a); // sends register address to read from
  Wire.endTransmission(); // end transmission
  
  Wire.requestFrom(_address, 1);// send data n-bytes read
  ret = Wire.read(); // receive DATA

  return ret;
}

uint32_t SensorDriverBmp085::read16(uint8_t a) {
  uint32_t ret;

  Wire.beginTransmission(_address); // start transmission to device 
  Wire.write(a); // sends register address to read from
  Wire.endTransmission(); // end transmission
  
  Wire.requestFrom(_address, 2);// send data n-bytes read
  ret = Wire.read(); // receive DATA
  ret <<= 8;
  ret |= Wire.read(); // receive DATA

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

uint32_t SensorDriverBmp085::readRawTemperature(void) {
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

float SensorDriverBmp085::readTemperature(void) {
  int32_t UT, B5;     // following ds convention
  float temp;

  UT = readRawTemperature();

#if BMP085_DEBUG == 1
  // use datasheet numbers!
  UT = 27898;
  ac6 = 23153;
  ac5 = 32757;
  mc = -8711;
  md = 2868;
#endif

  B5 = computeB5(UT);
  temp = (B5+8) >> 4;
  temp /= 10;
  return temp;

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

int SensorDriverBmp085::prepare(uint32_t& waittime)
{

  _timing=millis();
  waittime= 10ul;

  return SD_SUCCESS;

}

int SensorDriverBmp085::get(uint32_t values[],size_t lenvalues)
{

  if (millis() - _timing > MAXDELAYFORREAD)     return SD_INTERNAL_ERROR;

  if (lenvalues>=1) values[0] = (uint32_t) round(float(readPressure())/10.);
  if (lenvalues>=2) values[1] = (uint32_t) round(readTemperature() * 100. + 27315.);

  _timing=0;

  return SD_SUCCESS;

}

#if defined (USEGETDATA)
int SensorDriverBmp085::getdata(uint32_t& data,uint16_t& width)
{
  data=0xFFFFFFFF;
  width=0xFFFF;
  return SD_INTERNAL_ERROR;
}
#endif

#if defined(USEAJSON)
aJsonObject* SensorDriverBmp085::getJson()
{
  uint32_t values[2];
  aJsonObject* jsonvalues;
  jsonvalues = aJson.createObject();
  //if (SensorDriverBmp085::get2(&pressure,&temperature) == SD_SUCCESS){
  if (SensorDriverBmp085::get(values,2) == SD_SUCCESS){
    // pressure
    aJson.addNumberToObject(jsonvalues, "B10004", (int32_t)values[0]);      
#if defined(SECONDARYPARAMETER)
    // temperature
    aJson.addNumberToObject(jsonvalues, "B12101", (int32_t)values[1]);      
#endif

  }else{
    aJson.addNullToObject(jsonvalues, "B10004");
#if defined(SECONDARYPARAMETER)
    aJson.addNullToObject(jsonvalues, "B12101");
#endif
  }
  return jsonvalues;
}
#endif

#if defined(USEARDUINOJSON)
int SensorDriverBmp085::getJson(char *json_buffer, size_t json_buffer_length)
{
  uint32_t values[2];
  StaticJsonDocument<200> doc;

  if (get(values,2) == SD_SUCCESS){
    if ((uint32_t)values[0] != 0xFFFFFFFF){
      doc["B10004"]= values[0];
    }else{
      doc["B12101"]=serialized("null");
    }

#if defined(SECONDARYPARAMETER)
    if ((uint32_t) values[1] != 0xFFFFFFFF){
      doc["B10004"]= values[1];
    }else{
      doc["B12101"]=serialized("null");
    }
#endif
  }else{
    doc["B10004"]=serialized("null");
#if defined(SECONDARYPARAMETER)
    doc["B12101"]=serialized("null");
#endif
  }

  serializeJson(doc,json_buffer, json_buffer_length);
  return SD_SUCCESS;
}
#endif
#endif


#if defined (HI7021DRIVER)
int SensorDriverSI7021::setup(const char* driver, const int address, const int node, const char* type
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

int SensorDriverSI7021::prepare(uint32_t& waittime)
{
  Wire.begin();
  Wire.beginTransmission(_address);
  Wire.endTransmission();
  delay(300);
  _timing=millis();
  waittime= 50ul;
  return SD_SUCCESS;

}

int SensorDriverSI7021::get(uint32_t values[],size_t lenvalues)
{

  if (millis() - _timing > MAXDELAYFORREAD)     return SD_INTERNAL_ERROR;

  unsigned int data[2];
  Wire.beginTransmission(_address);
  Wire.write(0xF5);
  Wire.endTransmission();
  delay(500);

  Wire.requestFrom(_address, 2);

  if(Wire.available() == 2)
  {
    data[0] = Wire.read();
    data[1] = Wire.read();
  }

  float humidity  = ((data[0] * 256.0) + data[1]);
  humidity = ((125 * humidity) / 65536.0) - 6;

  Wire.beginTransmission(_address);
  Wire.write(0xF3);
  Wire.endTransmission();

  delay(500);

  Wire.requestFrom(_address, 2);

  if(Wire.available() == 2)
  {
    data[0] = Wire.read();
    data[1] = Wire.read();
  }

  uint32_t temperature  = ((data[0] * 256.0) + data[1]);
  temperature =  (((175.72 * float(temperature)) / 65536.0) - 46.85) * 100. + 27315.;

  if (lenvalues >= 1)  values[0] = round (humidity);
  if (lenvalues >= 2)  values[1] = round(temperature);
  _timing=0;
  return SD_SUCCESS;

}

#if defined (USEGETDATA)
int SensorDriverSI7021::getdata(uint32_t& data,uint16_t& width)
{
  data=0xFFFFFFFF;
  width=0xFFFF;
  return SD_INTERNAL_ERROR;
}
#endif

#if defined(USEAJSON)
aJsonObject* SensorDriverSI7021::getJson()
{
  uint32_t values[2];

  aJsonObject* jsonvalues;
  jsonvalues = aJson.createObject();
  //if (SensorDriverTmp::get2(&humidity,&temperature) == SD_SUCCESS){
  if (SensorDriverSI7021::get(values,2) == SD_SUCCESS){
    aJson.addNumberToObject(jsonvalues, "B13003", (int32_t)values[0]);      

#if defined(SECONDARYPARAMETER)
    // if you have a second value add here
    aJson.addNumberToObject(jsonvalues, "B12101", (int32_t)values[1]);      
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

int SensorDriverDw1::prepare(uint32_t& waittime)
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

int SensorDriverDw1::get(uint32_t values[],size_t lenvalues)
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

#if defined (USEGETDATA)
int SensorDriverDw1::getdata(uint32_t& data,uint16_t& width)
{
  data=0xFFFFFFFF;
  width=0xFFFF;
  return SD_INTERNAL_ERROR;
}
#endif

  #if defined(USEAJSON)
aJsonObject* SensorDriverDw1::getJson()
{
  uint32_t values[2];

  aJsonObject* jsonvalues;
  jsonvalues = aJson.createObject();
  if (SensorDriverDw1::get(values,2) == SD_SUCCESS){
    if (values[0] >= 0){
      aJson.addNumberToObject(jsonvalues, "B11001", (int32_t)values[0]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B11001");
    }
    // if you have a second value add here
    if (values[1] >= 0){
      aJson.addNumberToObject(jsonvalues, "B11002", (int32_t)values[1]);      
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

int SensorDriverTbr::prepare(uint32_t& waittime)
{

  Wire.beginTransmission(_address);
  Wire.write(I2C_RAIN_COMMAND);
  Wire.write(I2C_RAIN_COMMAND_STARTSTOP);
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  _timing=millis();
  waittime= 1ul;

  return SD_SUCCESS;
}

int SensorDriverTbr::get(uint32_t values[],size_t lenvalues)
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

#if defined (USEGETDATA)
int SensorDriverTbr::getdata(uint32_t& data,uint16_t& width)
{
  data=0xFFFFFFFF;
  width=0xFFFF;
  return SD_INTERNAL_ERROR;
}
#endif

#if defined(USEAJSON)
aJsonObject* SensorDriverTbr::getJson()
{
  uint32_t values[1];

  aJsonObject* jsonvalues;
  jsonvalues = aJson.createObject();
  if (SensorDriverTbr::get(values,1) == SD_SUCCESS){
    if (values[0] >= 0){
      aJson.addNumberToObject(jsonvalues, "B13011", (int32_t)values[0]);      
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

int SensorDriverTHoneshot::prepare(uint32_t& waittime)
{

  Wire.beginTransmission(_address);
  Wire.write(I2C_TH_COMMAND);
  Wire.write(I2C_TH_COMMAND_ONESHOT_START);
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  _timing=millis();
  waittime= 500ul;

  return SD_SUCCESS;
}

int SensorDriverTHoneshot::get(uint32_t values[],size_t lenvalues)
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

#if defined (USEGETDATA)
int SensorDriverTHoneshot::getdata(uint32_t& data,uint16_t& width)
{
  data=0xFFFFFFFF;
  width=0xFFFF;
  return SD_INTERNAL_ERROR;
}
#endif

#if defined(USEAJSON)
aJsonObject* SensorDriverTHoneshot::getJson()
{
  uint32_t values[2];

  aJsonObject* jsonvalues;
  jsonvalues = aJson.createObject();
  if (SensorDriverTHoneshot::get(values,2) == SD_SUCCESS){
    if (values[0] >= 0){
      aJson.addNumberToObject(jsonvalues, "B12101", (int32_t)values[0]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B12101");
    }

    if (values[1] >= 0){
      aJson.addNumberToObject(jsonvalues, "B13003", (int32_t)values[1]);      
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

int SensorDriverTH60mean::prepare(uint32_t& waittime)
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

int SensorDriverTH60mean::get(uint32_t values[],size_t lenvalues)
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

#if defined (USEGETDATA)
int SensorDriverTH60mean::getdata(uint32_t& data,uint16_t& width)
{
  data=0xFFFFFFFF;
  width=0xFFFF;
  return SD_INTERNAL_ERROR;
}
#endif

#if defined(USEAJSON)
aJsonObject* SensorDriverTH60mean::getJson()
{
  uint32_t values[2];

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
      aJson.addNumberToObject(jsonvalues, "B12101", (int32_t)values[0]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B12101");
    }
    if (values[1] >= 0){
      aJson.addNumberToObject(jsonvalues, "B13003", (int32_t)values[1]);      
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

int SensorDriverTHmean::prepare(uint32_t& waittime)
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

int SensorDriverTHmean::get(uint32_t values[],size_t lenvalues)
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

#if defined (USEGETDATA)
int SensorDriverTHmean::getdata(uint32_t& data,uint16_t& width)
{
  data=0xFFFFFFFF;
  width=0xFFFF;
  return SD_INTERNAL_ERROR;
}
#endif

#if defined(USEAJSON)
aJsonObject* SensorDriverTHmean::getJson()
{
  uint32_t values[2];

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
      aJson.addNumberToObject(jsonvalues, "B12101", (int32_t)values[0]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B12101");
    }

    if (values[1] >= 0){
      aJson.addNumberToObject(jsonvalues, "B13003", (int32_t)values[1]);      
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

int SensorDriverTHmin::prepare(uint32_t& waittime)
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

int SensorDriverTHmin::get(uint32_t values[],size_t lenvalues)
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

#if defined (USEGETDATA)
int SensorDriverTHmean::getdata(uint32_t& data,uint16_t& width)
{
  data=0xFFFFFFFF;
  width=0xFFFF;
  return SD_INTERNAL_ERROR;
}
#endif

#if defined(USEAJSON)
aJsonObject* SensorDriverTHmin::getJson()
{
  uint32_t values[2];

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
      aJson.addNumberToObject(jsonvalues, "B12101", (int32_t)values[0]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B12101");
    }
    if (values[1] >= 0){
      aJson.addNumberToObject(jsonvalues, "B13003", (int32_t)values[1]);      
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

int SensorDriverTHmax::prepare(uint32_t& waittime)
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

int SensorDriverTHmax::get(uint32_t values[],size_t lenvalues)
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

#if defined (USEGETDATA)
int SensorDriverTHmax::getdata(uint32_t& data,uint16_t& width)
{
  data=0xFFFFFFFF;
  width=0xFFFF;
  return SD_INTERNAL_ERROR;
}
#endif

#if defined(USEAJSON)
aJsonObject* SensorDriverTHmax::getJson()
{
  uint32_t values[2];

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
      aJson.addNumberToObject(jsonvalues, "B12101", (int32_t)values[0]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B12101");
    }

    if (values[1] >= 0){
      aJson.addNumberToObject(jsonvalues, "B13003", (int32_t)values[1]);      
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
  Wire.write(I2C_SDSMICS_ONESHOT);
  Wire.write(oneshot);
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  delay(10);

  Wire.beginTransmission(_address);
  Wire.write(I2C_SDSMICS_COMMAND);
  Wire.write(I2C_SDSMICS_COMMAND_ONESHOT_STOP);
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  return SD_SUCCESS;

}

int SensorDriverSDS011oneshot::prepare(uint32_t& waittime)
{

  if (! SDSMICSstarted) {
    Wire.beginTransmission(_address);
    Wire.write(I2C_SDSMICS_COMMAND);
    Wire.write(I2C_SDSMICS_COMMAND_ONESHOT_START);
    if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

    SDSMICSstarted=true;

#ifdef ONESHOT_SWITCHOFF
    //if you stop measure every get
    waittime= 19500ul;
#else
    //if you do not stop measure every get
    waittime= 3000ul;
#endif

  }else{
    waittime= 1ul;
  }

  _timing=millis();
  return SD_SUCCESS;
}

int SensorDriverSDS011oneshot::get(uint32_t values[],size_t lenvalues)
{
  unsigned char msb, lsb;
  if (millis() - _timing > MAXDELAYFORREAD)     return SD_INTERNAL_ERROR;

#ifdef ONESHOT_SWITCHOFF
  if (SDSMICSstarted) {
    // command STOP
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_SDSMICS_COMMAND);
    Wire.write(I2C_SDSMICS_COMMAND_ONESHOT_STOP);
    if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

    SDSMICSstarted=false;
    delay(100);
  }
#endif

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

#if defined (USEGETDATA)
int SensorDriverSDS011oneshot::getdata(uint32_t& data,uint16_t& width)
{
  data=0xFFFFFFFF;
  width=0xFFFF;
  return SD_INTERNAL_ERROR;
}
#endif

#if defined(USEAJSON)
aJsonObject* SensorDriverSDS011oneshot::getJson()
{
  uint32_t values[2];

  aJsonObject* jsonvalues;
  jsonvalues = aJson.createObject();
  if (SensorDriverSDS011oneshot::get(values,2) == SD_SUCCESS){
    if (values[0] >= 0){
      aJson.addNumberToObject(jsonvalues, "B15198", (int32_t)values[0]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B15198");
    }

    if (values[1] >= 0){
      aJson.addNumberToObject(jsonvalues, "B15195", (int32_t)values[1]);      
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
#if defined(USEARDUINOJSON)
int SensorDriverSDS011oneshot::getJson(char *json_buffer, size_t json_buffer_length)
{
  uint32_t values[2];
  StaticJsonDocument<200> doc;

  if (get(values,2) == SD_SUCCESS){
    if (values[0] >= 0){
      doc["B15198"]= values[0];      
    }else{
      doc["B15198"]=serialized("null");
    }

    if (values[1] >= 0){
      doc["B15195"]= values[1];
    }else{
      doc["B15195"]=serialized("null");
    }

  }else{
    doc["B15198"]=serialized("null");
    doc["B15195"]=serialized("null");
  }

  serializeJson(doc,json_buffer, json_buffer_length);
  return SD_SUCCESS;
}
#endif

#if defined (SDS011_LOCALSERIAL)
// serial driver for SDS011

/*
SensorDriverSDS011oneshotSerial::SensorDriverSDS011oneshotSerial(){
  //sdsSerial= new SoftwareSerial(2,3, false, 128);
  _sds011 = new sds011::Sds011(*sdsSerial);
  }
*/

int SensorDriverSDS011oneshotSerial::setup(const char* driver, const int address, const int node, const char* type)
{

  SensorDriver::setup(driver,address,node,type);
  //bool oneshot=true;

   #if defined(ARDUINO_ARCH_ESP8266)
  _sdsSerial=new SoftwareSerial();
  _sdsSerial->begin(9600,SWSERIAL_8N1, SDS_PIN_RX, SDS_PIN_TX, false, 128,11);
   #else
  _sdsSerial=new HardwareSerial(1);
  _sdsSerial->begin(9600,SERIAL_8N1, SDS_PIN_RX, SDS_PIN_TX, false, 128,11);
   #endif
  _sds011 = new sds011::Sds011(*_sdsSerial);
  delay(1000);
 
  /*
  switch (address)
    {
    case 0:                  // Serial 0
      Serial.begin(9600);
      _sds011 = sds011::Sds011(Serial); 
      break;
    case 1:                  // Serial 1
      Serial1.begin(9600);
      _sds011 = sds011::Sds011(Serial1);   
      break;
    case default:                 // software serial with pins defined by address
      if (address > 10){
	SoftwareSerial mySerial(address/10,address%10);
	mySerial.begin(9600);
	_sds011 = sds011::Sds011(myserial);   
      }else{
	return SD_INTERNAL_ERROR;
      }
      break;
    }
  
  */

  //sdsSerial->begin(9600);

  //_sds011->set_sleep(false);
  IF_SDSDEBUG(SDDBGSERIAL.print(F("Sds011 firmware version: ")));
  IF_SDSDEBUG(SDDBGSERIAL.println(_sds011->firmware_version()));

  if (_sds011->set_mode(sds011::QUERY)){
    //if (_sds011->set_sleep(sds011::SLEEP)){

      SDSMICSstarted=false;
      _timing=millis();

      return SD_SUCCESS;
      //}
  }
  return SD_INTERNAL_ERROR;
}

int SensorDriverSDS011oneshotSerial::prepare(uint32_t& waittime)
{
  //if (_sds011->set_sleep(sds011::WORK)) {
    SDSMICSstarted=true;
    _timing=millis();
    //waittime= 30000ul;
    waittime= 10ul;
    return SD_SUCCESS;
    //}else{
    //return SD_INTERNAL_ERROR;
    //}
}

int SensorDriverSDS011oneshotSerial::get(uint32_t values[],size_t lenvalues)
{
  int pm25=0xFFFFFFFF;
  int pm10=0xFFFFFFFF;

  if (millis() - _timing > MAXDELAYFORREAD) return SD_INTERNAL_ERROR;
  if (!SDSMICSstarted)  return SD_INTERNAL_ERROR;

  SDSMICSstarted=false;  
  _timing=0;
  
  if (_sds011->query_data_auto(&pm25, &pm10, SDSSAMPLES)) {
    // get pm25
    if (lenvalues >= 1) {
      values[0] = pm25 ;
    //if (values[0] == 0 ) return SD_INTERNAL_ERROR;
    }
    
    // get pm10
    if (lenvalues >= 2) {
      values[1] = pm10 ;
      //if (values[1] == 0 ) return SD_INTERNAL_ERROR;
    }
  }else{
    values[0]=0xFFFFFFFF;
    values[1]=0xFFFFFFFF;
    //_sds011->set_sleep(sds011::SLEEP);
    return SD_INTERNAL_ERROR;
  }

  //_sds011->set_sleep(sds011::SLEEP);
  return SD_SUCCESS;

}

#if defined (USEGETDATA)
int SensorDriverSDS011oneshotSerial::getdata(uint32_t& data,uint8_t& width)
{
  data=0xFFFFFFFF;
  width=0xFFFF;
  return SD_INTERNAL_ERROR;
}
#endif

#if defined(USEAJSON)
aJsonObject* SensorDriverSDS011oneshotSerial::getJson()
{
  uint32_t values[2];

  aJsonObject* jsonvalues;
  jsonvalues = aJson.createObject();
  if (SensorDriverSDS011oneshotSerial::get(values,2) == SD_SUCCESS){
    if (values[0] != 0xFFFFFFFF){
      aJson.addNumberToObject(jsonvalues, "B15198", (int32_t)values[0]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B15198");
    }

    if (values[1] != 0xFFFFFFFF){
      aJson.addNumberToObject(jsonvalues, "B15195", (int32_t)values[1]);      
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

#if defined(USEARDUINOJSON)
int SensorDriverSDS011oneshotSerial::getJson(char *json_buffer, size_t json_buffer_length)
{
  uint32_t values[2];
  StaticJsonDocument<200> doc;

  if (get(values,2) == SD_SUCCESS){
    if ((uint32_t)values[0] != 0xFFFFFFFF){
      doc["B15198"]= values[0];      
    }else{
      doc["B15198"]=serialized("null");
    }

    if ((uint32_t) values[1] != 0xFFFFFFFF){
      doc["B15195"]= values[1];
    }else{
      doc["B15195"]=serialized("null");
    }

  }else{
    doc["B15198"]=serialized("null");
    doc["B15195"]=serialized("null");
  }

  serializeJson(doc,json_buffer, json_buffer_length);
  return SD_SUCCESS;
}
#endif

//destructor
SensorDriverSDS011oneshotSerial::~SensorDriverSDS011oneshotSerial(){

  delete _sds011;
  //warning: deleting object of polymorphic class type 'SoftwareSerial' which has non-virtual destructor might cause undefined behaviour [-Wdelete-non-virtual-dtor]
  //delete _sdsSerial;
}
#endif

#endif

#if defined (SDS011_REPORT)


int SensorDriverSDS01160mean::setup(const char* driver, const int address, const int node, const char* type
  #if defined (RADIORF24)
			   , char* mainbuf, size_t lenbuf, RF24Network* network
    #if defined (AES)
			   , uint8_t key[] , uint16_t iv[]
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
  Wire.write(I2C_SDSMICS_ONESHOT);
  Wire.write(oneshot);
 
  short unsigned int ntry=NTRY;
  while  (ntry>0 && Wire.endTransmission() != 0){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#SDS01160mean setup retry ")));
    ntry--;
    delay(1000);
    Wire.beginTransmission(_address);
    Wire.write(I2C_SDSMICS_ONESHOT);
    Wire.write(oneshot);
  }
  if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  // command START
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_SDSMICS_COMMAND);
  Wire.write(I2C_SDSMICS_COMMAND_STOP);
  ntry=NTRY;
  while  (ntry>0 && Wire.endTransmission() != 0){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#SDS01160mean setup retry ")));
    ntry--;
    delay(1000);
    // command START
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_SDSMICS_COMMAND);
    Wire.write(I2C_SDSMICS_COMMAND_STOP);
  }
  if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  return SD_SUCCESS;

}

int SensorDriverSDS01160mean::prepare(uint32_t& waittime)
{

  if (SDS011counter < 0) SDS011counter=0;
  SDS011counter++;
  _timing=millis();

  if (SDS011counter == 1) {
    // This driver should be the fist of the SDS011 serie; we need to send COMMAND_STOP one time only !
    // command STOP
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_SDSMICS_COMMAND);
    Wire.write(I2C_SDSMICS_COMMAND_STOP);
    short unsigned int ntry=NTRY;
    while  (ntry>0 && Wire.endTransmission() != 0){
      IF_SDSDEBUG(SDDBGSERIAL.println(F("#SDS01160mean prepare retry ")));
      ntry--;
      delay(1000);
      Wire.beginTransmission(_address);   // Open I2C line in write mode
      Wire.write(I2C_SDSMICS_COMMAND);
      Wire.write(I2C_SDSMICS_COMMAND_STOP);
    }
    if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
    waittime= 100ul;
  }else{
    waittime= 1ul;
  }


  return SD_SUCCESS;
}

int SensorDriverSDS01160mean::get(uint32_t values[],size_t lenvalues)
{
  SDS011counter--;

  unsigned char msb, lsb;
  if (millis() - _timing > MAXDELAYFORREAD)     return SD_INTERNAL_ERROR;

  // get PM25
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_SDS011_MEANPM25);

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


  // get PM10
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_SDS011_MEANPM10);

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
    Wire.write(I2C_SDSMICS_COMMAND);
    Wire.write(I2C_SDSMICS_COMMAND_START);
    short unsigned int ntry=NTRY;
    while  (ntry>0 && Wire.endTransmission() != 0){
      IF_SDSDEBUG(SDDBGSERIAL.println(F("#SDS01160mean setup retry ")));
      ntry--;
      delay(1000);
      // command START
      Wire.beginTransmission(_address);   // Open I2C line in write mode
      Wire.write(I2C_SDSMICS_COMMAND);
      Wire.write(I2C_SDSMICS_COMMAND_START);
    }
    if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  }
  */

  _timing=0;

  return SD_SUCCESS;

}

#if defined (USEGETDATA)
int SensorDriverSDS011mean::getdata(uint32_t& data,uint16_t& width)
{
  data=0xFFFFFFFF;
  width=0xFFFF;
  return SD_INTERNAL_ERROR;
}
#endif

#if defined(USEAJSON)
aJsonObject* SensorDriverSDS01160mean::getJson()
{
  uint32_t values[2];

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
      aJson.addNumberToObject(jsonvalues, "B15198", (int32_t)values[0]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B15198");
    }
    if (values[1] >= 0){
      aJson.addNumberToObject(jsonvalues, "B15195", (int32_t)values[1]);      
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
  Wire.write(I2C_SDSMICS_ONESHOT);
  Wire.write(oneshot);
  short unsigned int ntry=NTRY;
  while  (ntry>0 && Wire.endTransmission() != 0){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#SDS011mean setup retry ")));
    ntry--;
    delay(1000);
    Wire.beginTransmission(_address);
    Wire.write(I2C_SDSMICS_ONESHOT);
    Wire.write(oneshot);
  }
  if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  // command START
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_SDSMICS_COMMAND);
  Wire.write(I2C_SDSMICS_COMMAND_STOP);
  ntry=NTRY;
  while  (ntry>0 && Wire.endTransmission() != 0){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#SDS01160mean setup retry ")));
    ntry--;
    delay(1000);
    // command START
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_SDSMICS_COMMAND);
    Wire.write(I2C_SDSMICS_COMMAND_STOP);
  }
  if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  return SD_SUCCESS;

}

int SensorDriverSDS011mean::prepare(uint32_t& waittime)
{

  if (SDS011counter < 0) SDS011counter=0;
  SDS011counter++;
  _timing=millis();

  if (SDS011counter == 1) {
    // This driver should be the fist of the SDS011 serie; we need to send COMMAND_STOP one time only !
    // command STOP
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_SDSMICS_COMMAND);
    Wire.write(I2C_SDSMICS_COMMAND_STOP);
    short unsigned int ntry=NTRY;
    while  (ntry>0 && Wire.endTransmission() != 0){
      IF_SDSDEBUG(SDDBGSERIAL.println(F("#SDS01160mean prepare retry ")));
      ntry--;
      delay(1000);
      Wire.beginTransmission(_address);   // Open I2C line in write mode
      Wire.write(I2C_SDSMICS_COMMAND);
      Wire.write(I2C_SDSMICS_COMMAND_STOP);
    }
    if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
    waittime= 100ul;
  }else{
    waittime= 1ul;
  }

  return SD_SUCCESS;
}

int SensorDriverSDS011mean::get(uint32_t values[],size_t lenvalues)
{
  SDS011counter--;

  unsigned char msb, lsb;
  if (millis() - _timing > MAXDELAYFORREAD)     return SD_INTERNAL_ERROR;

  // get PM25
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_SDS011_MEANPM25);

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

  // get PM10
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_SDS011_MEANPM10);

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
    Wire.write(I2C_SDSMICS_COMMAND);
    Wire.write(I2C_SDSMICS_COMMAND_START);
    short unsigned int ntry=NTRY;
    while  (ntry>0 && Wire.endTransmission() != 0){
      IF_SDSDEBUG(SDDBGSERIAL.println(F("#SDS01160mean setup retry ")));
      ntry--;
      delay(1000);
      // command START
      Wire.beginTransmission(_address);   // Open I2C line in write mode
      Wire.write(I2C_SDSMICS_COMMAND);
      Wire.write(I2C_SDSMICS_COMMAND_START);
    }
    if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  }
  */
  _timing=0;

  return SD_SUCCESS;

}

#if defined (USEGETDATA)
int SensorDriverSDS011mean::getdata(uint32_t& data,uint16_t& width)
{
  data=0xFFFFFFFF;
  width=0xFFFF;
  return SD_INTERNAL_ERROR;
}
#endif

#if defined(USEAJSON)
aJsonObject* SensorDriverSDS011mean::getJson()
{
  uint32_t values[2];

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
      aJson.addNumberToObject(jsonvalues, "B15198", (int32_t)values[0]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B15198");
    }

    if (values[1] >= 0){
      aJson.addNumberToObject(jsonvalues, "B15195", (int32_t)values[1]);      
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
  Wire.write(I2C_SDSMICS_ONESHOT);
  Wire.write(oneshot);
 
  short unsigned int ntry=NTRY;
  while  (ntry>0 && Wire.endTransmission() != 0){
    ntry--;
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#SDS011min setup retry ")));
    delay(1000);
    Wire.beginTransmission(_address);
    Wire.write(I2C_SDSMICS_ONESHOT);
    Wire.write(oneshot);
  }
  if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  // command START
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_SDSMICS_COMMAND);
  Wire.write(I2C_SDSMICS_COMMAND_STOP);
  ntry=NTRY;
  while  (ntry>0 && Wire.endTransmission() != 0){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#SDS01160mean setup retry ")));
    ntry--;
    delay(1000);
    // command START
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_SDSMICS_COMMAND);
    Wire.write(I2C_SDSMICS_COMMAND_STOP);
  }
  if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  return SD_SUCCESS;

}

int SensorDriverSDS011min::prepare(uint32_t& waittime)
{

  if (SDS011counter < 0) SDS011counter=0;
  SDS011counter++;
  _timing=millis();

  if (SDS011counter == 1) {
    // This driver should be the fist of the SDS011 serie; we need to send COMMAND_STOP one time only !
    // command STOP
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_SDSMICS_COMMAND);
    Wire.write(I2C_SDSMICS_COMMAND_STOP);
    short unsigned int ntry=NTRY;
    while  (ntry>0 && Wire.endTransmission() != 0){
      IF_SDSDEBUG(SDDBGSERIAL.println(F("#SDS01160mean prepare retry ")));
      ntry--;
      delay(1000);
      Wire.beginTransmission(_address);   // Open I2C line in write mode
      Wire.write(I2C_SDSMICS_COMMAND);
      Wire.write(I2C_SDSMICS_COMMAND_STOP);
    }
    if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
    waittime= 100ul;
  }else{
    waittime= 1ul;
  }

  return SD_SUCCESS;
}

int SensorDriverSDS011min::get(uint32_t values[],size_t lenvalues)
{
  SDS011counter--;
  unsigned char msb, lsb;
  if (millis() - _timing > MAXDELAYFORREAD)     return SD_INTERNAL_ERROR;

  // get PM25
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_SDS011_MINPM25);

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

  // get PM10
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_SDS011_MINPM10);

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
    Wire.write(I2C_SDSMICS_COMMAND);
    Wire.write(I2C_SDSMICS_COMMAND_START);
    short unsigned int ntry=NTRY;
    while  (ntry>0 && Wire.endTransmission() != 0){
      IF_SDSDEBUG(SDDBGSERIAL.println(F("#SDS01160mean setup retry ")));
      ntry--;
      delay(1000);
      // command START
      Wire.beginTransmission(_address);   // Open I2C line in write mode
      Wire.write(I2C_SDSMICS_COMMAND);
      Wire.write(I2C_SDSMICS_COMMAND_START);
    }
    if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  }
  */

  _timing=0;

  return SD_SUCCESS;

}

#if defined (USEGETDATA)
int SensorDriverSDS011min::getdata(uint32_t& data,uint16_t& width)
{
  data=0xFFFFFFFF;
  width=0xFFFF;
  return SD_INTERNAL_ERROR;
}
#endif

#if defined(USEAJSON)
aJsonObject* SensorDriverSDS011min::getJson()
{
  uint32_t values[2];

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
      aJson.addNumberToObject(jsonvalues, "B15198", (int32_t)values[0]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B15198");
    }
    if (values[1] >= 0){
      aJson.addNumberToObject(jsonvalues, "B15195", (int32_t)values[1]);      
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
  Wire.write(I2C_SDSMICS_ONESHOT);
  Wire.write(oneshot);
  short unsigned int ntry=NTRY;
  while  (ntry>0 && Wire.endTransmission() != 0){
    ntry--;
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#SDS011max setup retry ")));
    delay(1000);
    Wire.beginTransmission(_address);
    Wire.write(I2C_SDSMICS_ONESHOT);
    Wire.write(oneshot);
  }
  if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  // command START
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_SDSMICS_COMMAND);
  Wire.write(I2C_SDSMICS_COMMAND_STOP);
  ntry=NTRY;
  while  (ntry>0 && Wire.endTransmission() != 0){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#SDS01160mean setup retry ")));
    ntry--;
    delay(1000);
    // command START
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_SDSMICS_COMMAND);
    Wire.write(I2C_SDSMICS_COMMAND_STOP);
  }
  if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  return SD_SUCCESS;

}

int SensorDriverSDS011max::prepare(uint32_t& waittime)
{

  if (SDS011counter < 0) SDS011counter=0;
  SDS011counter++;
  _timing=millis();
  if (SDS011counter == 1) {
    // This driver should be the fist of the SDS011 serie; we need to send COMMAND_STOP one time only !
    // command STOP
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_SDSMICS_COMMAND);
    Wire.write(I2C_SDSMICS_COMMAND_STOP);
    short unsigned int ntry=NTRY;
    while  (ntry>0 && Wire.endTransmission() != 0){
      IF_SDSDEBUG(SDDBGSERIAL.println(F("#SDS01160mean prepare retry ")));
      ntry--;
      delay(1000);
      Wire.beginTransmission(_address);   // Open I2C line in write mode
      Wire.write(I2C_SDSMICS_COMMAND);
      Wire.write(I2C_SDSMICS_COMMAND_STOP);
    }
    if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
    waittime= 100ul;
  }else{
    waittime= 1ul;
  }

  return SD_SUCCESS;
}

int SensorDriverSDS011max::get(uint32_t values[],size_t lenvalues)
{
  SDS011counter--;
  unsigned char msb, lsb;
  if (millis() - _timing > MAXDELAYFORREAD)     return SD_INTERNAL_ERROR;

  // get PM25
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_SDS011_MAXPM25);

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

  // get PM10
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_SDS011_MAXPM10);

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
    Wire.write(I2C_SDSMICS_COMMAND);
    Wire.write(I2C_SDSMICS_COMMAND_START);
    short unsigned int ntry=NTRY;
    while  (ntry>0 && Wire.endTransmission() != 0){
      IF_SDSDEBUG(SDDBGSERIAL.println(F("#SDS01160mean setup retry ")));
      ntry--;
      delay(1000);
      // command START
      Wire.beginTransmission(_address);   // Open I2C line in write mode
      Wire.write(I2C_SDSMICS_COMMAND);
      Wire.write(I2C_SDSMICS_COMMAND_START);
    }
    if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  }
  */

  _timing=0;

  return SD_SUCCESS;

}

#if defined (USEGETDATA)
int SensorDriverSDS011max::getdata(uint32_t& data,uint16_t& width)
{
  data=0xFFFFFFFF;
  width=0xFFFF;
  return SD_INTERNAL_ERROR;
}
#endif

#if defined(USEAJSON)
aJsonObject* SensorDriverSDS011max::getJson()
{
  uint32_t values[2];

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
      aJson.addNumberToObject(jsonvalues, "B15198", (int32_t)values[0]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B15198");
    }

    if (values[1] >= 0){
      aJson.addNumberToObject(jsonvalues, "B15195", (int32_t)values[1]);      
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


#if defined (MICS4514_ONESHOT)
int SensorDriverMICS4514oneshot::setup(const char* driver, const int address, const int node, const char* type
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
  Wire.write(I2C_SDSMICS_ONESHOT);
  Wire.write(oneshot);
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  delay(10);

  Wire.beginTransmission(_address);
  Wire.write(I2C_SDSMICS_COMMAND);
  Wire.write(I2C_SDSMICS_COMMAND_ONESHOT_STOP);
  if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  return SD_SUCCESS;

}

int SensorDriverMICS4514oneshot::prepare(uint32_t& waittime)
{

  if (! SDSMICSstarted) {
    Wire.beginTransmission(_address);
    Wire.write(I2C_SDSMICS_COMMAND);
    Wire.write(I2C_SDSMICS_COMMAND_ONESHOT_START);
    if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

    waittime= 19500ul;
  }else{
    waittime= 1ul;
  }

  _timing=millis();
  return SD_SUCCESS;
}

int SensorDriverMICS4514oneshot::get(uint32_t values[],size_t lenvalues)
{
  unsigned char msb, lsb;
  if (millis() - _timing > MAXDELAYFORREAD)     return SD_INTERNAL_ERROR;

  if (SDSMICSstarted) {
    // command STOP
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_SDSMICS_COMMAND);
    Wire.write(I2C_SDSMICS_COMMAND_ONESHOT_STOP);
    if (Wire.endTransmission() != 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
    SDSMICSstarted=false;
    delay(100);
  }

  // get CO
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_MICS4514_CO);
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

  // get NO2
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_MICS4514_NO2);
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

#if defined (USEGETDATA)
int SensorDriverMICS4514oneshot::getdata(uint32_t& data,uint16_t& width)
{
  data=0xFFFFFFFF;
  width=0xFFFF;
  return SD_INTERNAL_ERROR;
}
#endif

#if defined(USEAJSON)
aJsonObject* SensorDriverMICS4514oneshot::getJson()
{
  uint32_t values[2];

  aJsonObject* jsonvalues;
  jsonvalues = aJson.createObject();
  if (SensorDriverMICS4514oneshot::get(values,2) == SD_SUCCESS){
    if (values[0] >= 0){
      aJson.addNumberToObject(jsonvalues, "B15196", (int32_t)values[0]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B15196");
    }

    if (values[1] >= 0){
      aJson.addNumberToObject(jsonvalues, "B15193", (int32_t)values[1]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B15193");
    }

  }else{
    aJson.addNullToObject(jsonvalues, "B15196");
    aJson.addNullToObject(jsonvalues, "B15193");
  }
  return jsonvalues;
}
#endif

#if defined(USEARDUINOJSON)
int SensorDriverMICS4514oneshot::getJson(char *json_buffer, size_t json_buffer_length)
{
  uint32_t values[2];
  StaticJsonDocument<200> doc;

  if (get(values,2) == SD_SUCCESS){
    if (values[0] >= 0){
      doc["B15196"]= values[0];      
    }else{
      doc["B15196"]=serialized("null");
    }

    if (values[1] >= 0){
      doc["B15193"]= values[1];
    }else{
      doc["B15193"]=serialized("null");
    }

  }else{
    doc["B15196"]=serialized("null");
    doc["B15193"]=serialized("null");
  }

  serializeJson(doc,json_buffer, json_buffer_length);
  return SD_SUCCESS;
}
#endif
#endif

#if defined (MICS4514_REPORT)


int SensorDriverMICS451460mean::setup(const char* driver, const int address, const int node, const char* type
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
  Wire.write(I2C_SDSMICS_ONESHOT);
  Wire.write(oneshot);
 
  short unsigned int ntry=NTRY;
  while  (ntry>0 && Wire.endTransmission() != 0){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#MICS451460mean setup retry ")));
    ntry--;
    delay(1000);
    Wire.beginTransmission(_address);
    Wire.write(I2C_SDSMICS_ONESHOT);
    Wire.write(oneshot);
  }
  if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  // command START
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_SDSMICS_COMMAND);
  Wire.write(I2C_SDSMICS_COMMAND_STOP);
  ntry=NTRY;
  while  (ntry>0 && Wire.endTransmission() != 0){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#MICS451460mean setup retry ")));
    ntry--;
    delay(1000);
    // command START
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_SDSMICS_COMMAND);
    Wire.write(I2C_SDSMICS_COMMAND_STOP);
  }
  if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  return SD_SUCCESS;

}

int SensorDriverMICS451460mean::prepare(uint32_t& waittime)
{

  if (MICS4514counter < 0) MICS4514counter=0;
  MICS4514counter++;
  _timing=millis();

  if (MICS4514counter == 1) {
    // This driver should be the fist of the MICS4514 serie; we need to send COMMAND_STOP one time only !
    // command STOP
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_SDSMICS_COMMAND);
    Wire.write(I2C_SDSMICS_COMMAND_STOP);
    short unsigned int ntry=NTRY;
    while  (ntry>0 && Wire.endTransmission() != 0){
      IF_SDSDEBUG(SDDBGSERIAL.println(F("#MICS451460mean prepare retry ")));
      ntry--;
      delay(1000);
      Wire.beginTransmission(_address);   // Open I2C line in write mode
      Wire.write(I2C_SDSMICS_COMMAND);
      Wire.write(I2C_SDSMICS_COMMAND_STOP);
    }
    if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
    waittime= 100ul;
  }else{
    waittime= 1ul;
  }


  return SD_SUCCESS;
}

int SensorDriverMICS451460mean::get(uint32_t values[],size_t lenvalues)
{
  MICS4514counter--;

  unsigned char msb, lsb;
  if (millis() - _timing > MAXDELAYFORREAD)     return SD_INTERNAL_ERROR;

  // get CO
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_MICS4514_MEANCO);

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


  // get NO2
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_MICS4514_MEANNO2);

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
  if (MICS4514counter == 0) {
    // This driver should be the last of the MICS4514 serie; we need to send COMMAND_START one time only !
    // command START
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_SDSMICS_COMMAND);
    Wire.write(I2C_SDSMICS_COMMAND_START);
    short unsigned int ntry=NTRY;
    while  (ntry>0 && Wire.endTransmission() != 0){
      IF_SDSDEBUG(SDDBGSERIAL.println(F("#MICS451460mean setup retry ")));
      ntry--;
      delay(1000);
      // command START
      Wire.beginTransmission(_address);   // Open I2C line in write mode
      Wire.write(I2C_SDSMICS_COMMAND);
      Wire.write(I2C_SDSMICS_COMMAND_START);
    }
    if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  }
  */

  _timing=0;

  return SD_SUCCESS;

}

#if defined (USEGETDATA)
int SensorDriverMICS451460mean::getdata(uint32_t& data,uint16_t& width)
{
  data=0xFFFFFFFF;
  width=0xFFFF;
  return SD_INTERNAL_ERROR;
}
#endif

#if defined(USEAJSON)
aJsonObject* SensorDriverMICS451460mean::getJson()
{
  uint32_t values[2];

  aJsonObject* jsonvalues;
  jsonvalues = aJson.createObject();

  short unsigned int ntry=NTRY;

  while (ntry > 0 && SensorDriverMICS451460mean::get(values,2) != SD_SUCCESS){
    delay(1000);
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#MICS451460mean get retry ")));
    ntry--;
  }

  if (ntry > 0){
    if (values[0] >= 0){
      aJson.addNumberToObject(jsonvalues, "B15198", (int32_t)values[0]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B15198");
    }
    if (values[1] >= 0){
      aJson.addNumberToObject(jsonvalues, "B15195", (int32_t)values[1]);      
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


int SensorDriverMICS4514mean::setup(const char* driver, const int address, const int node, const char* type
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
  Wire.write(I2C_SDSMICS_ONESHOT);
  Wire.write(oneshot);
  short unsigned int ntry=NTRY;
  while  (ntry>0 && Wire.endTransmission() != 0){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#MICS4514mean setup retry ")));
    ntry--;
    delay(1000);
    Wire.beginTransmission(_address);
    Wire.write(I2C_SDSMICS_ONESHOT);
    Wire.write(oneshot);
  }
  if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  // command START
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_SDSMICS_COMMAND);
  Wire.write(I2C_SDSMICS_COMMAND_STOP);
  ntry=NTRY;
  while  (ntry>0 && Wire.endTransmission() != 0){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#MICS451460mean setup retry ")));
    ntry--;
    delay(1000);
    // command START
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_SDSMICS_COMMAND);
    Wire.write(I2C_SDSMICS_COMMAND_STOP);
  }
  if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  return SD_SUCCESS;

}

int SensorDriverMICS4514mean::prepare(uint32_t& waittime)
{

  if (MICS4514counter < 0) MICS4514counter=0;
  MICS4514counter++;
  _timing=millis();

  if (MICS4514counter == 1) {
    // This driver should be the fist of the MICS4514 serie; we need to send COMMAND_STOP one time only !
    // command STOP
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_SDSMICS_COMMAND);
    Wire.write(I2C_SDSMICS_COMMAND_STOP);
    short unsigned int ntry=NTRY;
    while  (ntry>0 && Wire.endTransmission() != 0){
      IF_SDSDEBUG(SDDBGSERIAL.println(F("#MICS451460mean prepare retry ")));
      ntry--;
      delay(1000);
      Wire.beginTransmission(_address);   // Open I2C line in write mode
      Wire.write(I2C_SDSMICS_COMMAND);
      Wire.write(I2C_SDSMICS_COMMAND_STOP);
    }
    if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
    waittime= 100ul;
  }else{
    waittime= 1ul;
  }

  return SD_SUCCESS;
}

int SensorDriverMICS4514mean::get(uint32_t values[],size_t lenvalues)
{
  MICS4514counter--;

  unsigned char msb, lsb;
  if (millis() - _timing > MAXDELAYFORREAD)     return SD_INTERNAL_ERROR;

  // get CO
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_MICS4514_MEANCO);

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

  // get NO2
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_MICS4514_MEANNO2);

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
  if (MICS4514counter == 0) {
    // This driver should be the last of the MICS4514 serie; we need to send COMMAND_START one time only !
    // command START
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_SDSMICS_COMMAND);
    Wire.write(I2C_SDSMICS_COMMAND_START);
    short unsigned int ntry=NTRY;
    while  (ntry>0 && Wire.endTransmission() != 0){
      IF_SDSDEBUG(SDDBGSERIAL.println(F("#MICS451460mean setup retry ")));
      ntry--;
      delay(1000);
      // command START
      Wire.beginTransmission(_address);   // Open I2C line in write mode
      Wire.write(I2C_SDSMICS_COMMAND);
      Wire.write(I2C_SDSMICS_COMMAND_START);
    }
    if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  }
  */
  _timing=0;

  return SD_SUCCESS;

}

#if defined (USEGETDATA)
int SensorDriverMICS4514mean::getdata(uint32_t& data,uint16_t& width)
{
  data=0xFFFFFFFF;
  width=0xFFFF;
  return SD_INTERNAL_ERROR;
}
#endif

#if defined(USEAJSON)
aJsonObject* SensorDriverMICS4514mean::getJson()
{
  uint32_t values[2];

  aJsonObject* jsonvalues;
  jsonvalues = aJson.createObject();

  short unsigned int ntry=NTRY;

  while (ntry > 0 && SensorDriverMICS4514mean::get(values,2) != SD_SUCCESS){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#MICS4514mean get retry ")));
    delay(1000);
    ntry--;
  }

  if (ntry > 0){
    if (values[0] >= 0){
      aJson.addNumberToObject(jsonvalues, "B15196", (int32_t)values[0]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B15196");
    }

    if (values[1] >= 0){
      aJson.addNumberToObject(jsonvalues, "B15193", (int32_t)values[1]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B15193");      
    }

  }else{
    aJson.addNullToObject(jsonvalues, "B15196");
    aJson.addNullToObject(jsonvalues, "B15193");
  }
  return jsonvalues;
}
#endif



int SensorDriverMICS4514min::setup(const char* driver, const int address, const int node, const char* type
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
  Wire.write(I2C_SDSMICS_ONESHOT);
  Wire.write(oneshot);
 
  short unsigned int ntry=NTRY;
  while  (ntry>0 && Wire.endTransmission() != 0){
    ntry--;
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#MICS4514min setup retry ")));
    delay(1000);
    Wire.beginTransmission(_address);
    Wire.write(I2C_SDSMICS_ONESHOT);
    Wire.write(oneshot);
  }
  if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  // command START
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_SDSMICS_COMMAND);
  Wire.write(I2C_SDSMICS_COMMAND_STOP);
  ntry=NTRY;
  while  (ntry>0 && Wire.endTransmission() != 0){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#MICS451460mean setup retry ")));
    ntry--;
    delay(1000);
    // command START
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_SDSMICS_COMMAND);
    Wire.write(I2C_SDSMICS_COMMAND_STOP);
  }
  if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  return SD_SUCCESS;

}

int SensorDriverMICS4514min::prepare(uint32_t& waittime)
{

  if (MICS4514counter < 0) MICS4514counter=0;
  MICS4514counter++;
  _timing=millis();

  if (MICS4514counter == 1) {
    // This driver should be the fist of the MICS4514 serie; we need to send COMMAND_STOP one time only !
    // command STOP
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_SDSMICS_COMMAND);
    Wire.write(I2C_SDSMICS_COMMAND_STOP);
    short unsigned int ntry=NTRY;
    while  (ntry>0 && Wire.endTransmission() != 0){
      IF_SDSDEBUG(SDDBGSERIAL.println(F("#MICS451460mean prepare retry ")));
      ntry--;
      delay(1000);
      Wire.beginTransmission(_address);   // Open I2C line in write mode
      Wire.write(I2C_SDSMICS_COMMAND);
      Wire.write(I2C_SDSMICS_COMMAND_STOP);
    }
    if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
    waittime= 100ul;
  }else{
    waittime= 1ul;
  }

  return SD_SUCCESS;
}

int SensorDriverMICS4514min::get(uint32_t values[],size_t lenvalues)
{
  MICS4514counter--;
  unsigned char msb, lsb;
  if (millis() - _timing > MAXDELAYFORREAD)     return SD_INTERNAL_ERROR;

  // get CO
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_MICS4514_MINCO);

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

  // get NO2
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_MICS4514_MINNO2);

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
  if (MICS4514counter == 0) {
    // This driver should be the last of the MICS4514 serie; we need to send COMMAND_START one time only !
    // command START
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_SDSMICS_COMMAND);
    Wire.write(I2C_SDSMICS_COMMAND_START);
    short unsigned int ntry=NTRY;
    while  (ntry>0 && Wire.endTransmission() != 0){
      IF_SDSDEBUG(SDDBGSERIAL.println(F("#MICS451460mean setup retry ")));
      ntry--;
      delay(1000);
      // command START
      Wire.beginTransmission(_address);   // Open I2C line in write mode
      Wire.write(I2C_SDSMICS_COMMAND);
      Wire.write(I2C_SDSMICS_COMMAND_START);
    }
    if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  }
  */

  _timing=0;

  return SD_SUCCESS;

}

#if defined (USEGETDATA)
int SensorDriverMICS4514min::getdata(uint32_t& data,uint16_t& width)
{
  data=0xFFFFFFFF;
  width=0xFFFF;
  return SD_INTERNAL_ERROR;
}
#endif

#if defined(USEAJSON)
aJsonObject* SensorDriverMICS4514min::getJson()
{
  uint32_t values[2];

  aJsonObject* jsonvalues;
  jsonvalues = aJson.createObject();

  short unsigned int ntry=NTRY;

  while (ntry > 0 && SensorDriverMICS4514min::get(values,2) != SD_SUCCESS){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#MICS4514min get retry ")));
    delay(1000);
    ntry--;
  }

  if (ntry > 0){
    if (values[0] >= 0){
      aJson.addNumberToObject(jsonvalues, "B15196", (int32_t)values[0]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B15196");
    }
    if (values[1] >= 0){
      aJson.addNumberToObject(jsonvalues, "B15193", (int32_t)values[1]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B15193");
    }

  }else{
    aJson.addNullToObject(jsonvalues, "B15196");
    aJson.addNullToObject(jsonvalues, "B15193");
  }
  return jsonvalues;
}
#endif

int SensorDriverMICS4514max::setup(const char* driver, const int address, const int node, const char* type
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
  Wire.write(I2C_SDSMICS_ONESHOT);
  Wire.write(oneshot);
  short unsigned int ntry=NTRY;
  while  (ntry>0 && Wire.endTransmission() != 0){
    ntry--;
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#MICS4514max setup retry ")));
    delay(1000);
    Wire.beginTransmission(_address);
    Wire.write(I2C_SDSMICS_ONESHOT);
    Wire.write(oneshot);
  }
  if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  // command START
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_SDSMICS_COMMAND);
  Wire.write(I2C_SDSMICS_COMMAND_STOP);
  ntry=NTRY;
  while  (ntry>0 && Wire.endTransmission() != 0){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#MICS451460mean setup retry ")));
    ntry--;
    delay(1000);
    // command START
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_SDSMICS_COMMAND);
    Wire.write(I2C_SDSMICS_COMMAND_STOP);
  }
  if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 

  return SD_SUCCESS;

}

int SensorDriverMICS4514max::prepare(uint32_t& waittime)
{

  if (MICS4514counter < 0) MICS4514counter=0;
  MICS4514counter++;
  _timing=millis();
  if (MICS4514counter == 1) {
    // This driver should be the fist of the MICS4514 serie; we need to send COMMAND_STOP one time only !
    // command STOP
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_SDSMICS_COMMAND);
    Wire.write(I2C_SDSMICS_COMMAND_STOP);
    short unsigned int ntry=NTRY;
    while  (ntry>0 && Wire.endTransmission() != 0){
      IF_SDSDEBUG(SDDBGSERIAL.println(F("#MICS451460mean prepare retry ")));
      ntry--;
      delay(1000);
      Wire.beginTransmission(_address);   // Open I2C line in write mode
      Wire.write(I2C_SDSMICS_COMMAND);
      Wire.write(I2C_SDSMICS_COMMAND_STOP);
    }
    if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
    waittime= 100ul;
  }else{
    waittime= 1ul;
  }

  return SD_SUCCESS;
}

int SensorDriverMICS4514max::get(uint32_t values[],size_t lenvalues)
{
  MICS4514counter--;
  unsigned char msb, lsb;
  if (millis() - _timing > MAXDELAYFORREAD)     return SD_INTERNAL_ERROR;

  // get CO
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_MICS4514_MAXCO);

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

  // get NO2
  Wire.beginTransmission(_address);   // Open I2C line in write mode
  Wire.write(I2C_SDS011_MAXNO2);

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
  if (MICS4514counter == 0) {
    // This driver should be the last of the MICS4514 serie; we need to send COMMAND_START one time only !
    // command START
    Wire.beginTransmission(_address);   // Open I2C line in write mode
    Wire.write(I2C_SDSMICS_COMMAND);
    Wire.write(I2C_SDSMICS_COMMAND_START);
    short unsigned int ntry=NTRY;
    while  (ntry>0 && Wire.endTransmission() != 0){
      IF_SDSDEBUG(SDDBGSERIAL.println(F("#MICS451460mean setup retry ")));
      ntry--;
      delay(1000);
      // command START
      Wire.beginTransmission(_address);   // Open I2C line in write mode
      Wire.write(I2C_SDSMICS_COMMAND);
      Wire.write(I2C_SDSMICS_COMMAND_START);
    }
    if (ntry == 0) return SD_INTERNAL_ERROR;             // End Write Transmission 
  }
  */

  _timing=0;

  return SD_SUCCESS;

}

#if defined (USEGETDATA)
int SensorDriverMICS4514max::getdata(uint32_t& data,uint16_t& width)
{
  data=0xFFFFFFFF;
  width=0xFFFF;
  return SD_INTERNAL_ERROR;
}
#endif

#if defined(USEAJSON)
aJsonObject* SensorDriverMICS4514max::getJson()
{
  uint32_t values[2];

  aJsonObject* jsonvalues;
  jsonvalues = aJson.createObject();

  short unsigned int ntry=NTRY;

  while (ntry > 0 && SensorDriverMICS4514max::get(values,2) != SD_SUCCESS){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#MICS4514max get retry ")));
    delay(1000);
    ntry--;
  }

  if (ntry > 0){
    if (values[0] >= 0){
      aJson.addNumberToObject(jsonvalues, "B15196", (int32_t)values[0]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B15196");
    }

    if (values[1] >= 0){
      aJson.addNumberToObject(jsonvalues, "B15193", (int32_t)values[1]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B15193");
    }

  }else{
    aJson.addNullToObject(jsonvalues, "B15196");
    aJson.addNullToObject(jsonvalues, "B15193");
  }
  return jsonvalues;
}
#endif
#endif


#if defined (HPM_ONESHOT)

int SensorDriverHPMoneshotSerial::setup(const char* driver, const int address, const int node, const char* type)
{

  SensorDriver::setup(driver,address,node,type);
  //bool oneshot=true;

  #if defined(ARDUINO_ARCH_ESP8266)
  _hpmSerial=new SoftwareSerial();
  _hpmSerial->begin(9600,SWSERIAL_8N1, HPM_PIN_RX, HPM_PIN_TX, false, 128,11);
  #else
  _hpmSerial=new HardwareSerial(1);
  _hpmSerial->begin(9600,SERIAL_8N1,HPM_PIN_RX, HPM_PIN_TX);
  //_hpmSerial= &Serial1;
  #endif

  _hpm = new hpm();
  delay(1000);

  IF_SDSDEBUG(SDDBGSERIAL.println(F("#try to build HPM")));
  
  if(_hpm->init(_hpmSerial)){
#ifdef ONESHOT_SWITCHOFF
    if( _hpm->stopParticleMeasurement()){
      HPMstarted=false;
#else
    if( _hpm->startParticleMeasurement()){
      HPMstarted=true;
#endif
      _timing=millis();

      return SD_SUCCESS;
    }
  }
  return SD_INTERNAL_ERROR;
}

int SensorDriverHPMoneshotSerial::prepare(uint32_t& waittime)
{
#ifdef ONESHOT_SWITCHOFF
  if(_hpm->startParticleMeasurement()){
    HPMstarted=true;
    _timing=millis();
    //waittime= 6000ul;
    waittime= 14500ul;
    return SD_SUCCESS;
  }else{
    return SD_INTERNAL_ERROR;
  }
#else
  _timing=millis();  
  waittime= 0ul;
  return SD_SUCCESS;  
#endif
}

int SensorDriverHPMoneshotSerial::get(uint32_t values[],size_t lenvalues)
{
  unsigned int pm25;
  unsigned int pm10;

  if (millis() - _timing > MAXDELAYFORREAD) return SD_INTERNAL_ERROR;
  if (!HPMstarted)  return SD_INTERNAL_ERROR;
  
  _timing=0;
  
  // measure


  
#ifdef ONESHOT_SWITCHOFF
  bool status = _hpm->query_data_auto( &pm25, &pm10, HPMSAMPLES);
  _hpm->stopParticleMeasurement();
  HPMstarted=false;  
#else
  bool status=_hpm->readParticleMeasuringResults();
  pm25 = _hpm->get(PM25_TYPE);
  pm10 = _hpm->get(PM10_TYPE);
#endif

  if (status){

  // get pm25
    if (lenvalues >= 1) {
      values[0] = pm25*10 ;
    }

    // get pm10
    if (lenvalues >= 2) {
      values[1] = pm10*10 ;
    }

    return SD_SUCCESS;
  } else {
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#hpm error")));
    return SD_INTERNAL_ERROR;    
  }
}

#if defined (USEGETDATA)
int SensorDriverHPMoneshotSerial::getdata(uint32_t& data,uint16_t& width)
{

  uint32_t values[1];
  width=20;
  const int16_t reference=0;
  
  if (SensorDriverHPMoneshotSerial::get(values,1) == SD_SUCCESS){
    data=(values[0]-reference);// << (sizeof(values[1])-width);
  }else{
    data=0xFFFFFFFF;
    width=0xFFFF;
    return SD_INTERNAL_ERROR;
  }
  return SD_SUCCESS;
  
}
#endif

#if defined(USEAJSON)
aJsonObject* SensorDriverHPMoneshotSerial::getJson()
{
  uint32_t values[2];

  aJsonObject* jsonvalues;
  jsonvalues = aJson.createObject();
  if (SensorDriverHPMoneshotSerial::get(values,2) == SD_SUCCESS){
    if (values[0] != 0xFFFFFFFF){
      aJson.addNumberToObject(jsonvalues, "B15198", (int32_t)values[0]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B15198");
    }

    if (values[1] != 0xFFFFFFFF){
      aJson.addNumberToObject(jsonvalues, "B15195", (int32_t)values[1]);      
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

#if defined(USEARDUINOJSON)
int SensorDriverHPMoneshotSerial::getJson(char *json_buffer, size_t json_buffer_length)
{
  uint32_t values[2];
  StaticJsonDocument<200> doc;

  if (get(values,2) == SD_SUCCESS){
    if ((uint32_t)values[0] != 0xFFFFFFFF){
      doc["B15198"]= values[0];      
    }else{
      doc["B15198"]=serialized("null");
    }

    if ((uint32_t) values[1] != 0xFFFFFFFF){
      doc["B15195"]= values[1];
    }else{
      doc["B15195"]=serialized("null");
    }

  }else{
    doc["B15198"]=serialized("null");
    doc["B15195"]=serialized("null");
  }

  serializeJson(doc,json_buffer, json_buffer_length);
  return SD_SUCCESS;
}
#endif

//destructor
SensorDriverHPMoneshotSerial::~SensorDriverHPMoneshotSerial(){

  delete _hpm;
  //warning: deleting object of polymorphic class type 'SoftwareSerial' which has non-virtual destructor might cause undefined behaviour [-Wdelete-non-virtual-dtor]
  //delete _hpmSerial;
}


#endif


#if defined (PMS_ONESHOT)

int SensorDriverPMSoneshotSerial::setup(const char* driver, const int address, const int node, const char* type)
{

  SensorDriver::setup(driver,address,node,type);
  //bool oneshot=true;

  #if defined(ARDUINO_ARCH_ESP8266)
  _pmsSerial=new SoftwareSerial();
  _pmsSerial->begin(9600,SWSERIAL_8N1, PMS_PIN_RX, PMS_PIN_TX, false, 128,11);
  #else
  _pmsSerial=new HardwareSerial(1);
  _pmsSerial->begin(9600,SERIAL_8N1,PMS_PIN_RX, PMS_PIN_TX);
  //_pmsSerial= &Serial1;
  #endif

  _pms = new Pmsx003();

  IF_SDSDEBUG(SDDBGSERIAL.println(F("#try to build PMS")));
  
  _pms->begin(_pmsSerial);
  _pms->cmd(Pmsx003::cmdWakeup);
  delay(2500);
  _pms->cmd(Pmsx003::cmdModePassive);
  delay(2500);
  
  PMSstarted=false;
  _timing=millis();

  return SD_SUCCESS;
}

int SensorDriverPMSoneshotSerial::prepare(uint32_t& waittime)
{
  _pms->cmd(Pmsx003::cmdReadData);
  
  PMSstarted=true;
  _timing=millis();
  //waittime= 6000ul;
  waittime= 2500ul;
  return SD_SUCCESS;
}

int SensorDriverPMSoneshotSerial::get(uint32_t values[],size_t lenvalues)
{
  bool status = false;
  uint16_t data[13];

  if (millis() - _timing > MAXDELAYFORREAD) return SD_INTERNAL_ERROR;
  if (!PMSstarted)  return SD_INTERNAL_ERROR;

  PMSstarted=false;  
  _timing=0;

  // measure
  while (_pms->available()){

    Pmsx003::PmsStatus pstatus = _pms->read(data, 13);
    
    switch (pstatus) {
    case Pmsx003::OK:
     
      status = true; 
      //for (uint8_t i = 0; i < 13; ++i) {
      //Serial.println(data[i]);
      //}
      break;
    default:
      //Serial.print(F("Error status: "));
      //Serial.println(status);
      goto endwhile;   // skip other broken messages
    }
  }
 endwhile: ;

  if (status){
    
    // get pm25
    if (lenvalues >= 1) {
      values[0] = data[4]*10 ;
    }

    // get pm10
    if (lenvalues >= 2) {
      values[1] = data[5]*10 ;
    }

    // get pm1
    if (lenvalues >= 3) {
      values[2] = data[3]*10 ;
    }

    // Data 6  indicates the number of particles with diameter beyond 0.3  um in 0.1 L of air.
    if (lenvalues >= 4) {
      values[3] = data[6] ;
    }
    
    // Data 7  indicates the number of particles with diameter beyond 0.5  um in 0.1 L of air.
    if (lenvalues >= 5) {
      values[4] = data[7] ;
    }
    
    // Data 8  indicates the number of particles with diameter beyond 1.0  um in 0.1 L of air.
    if (lenvalues >= 6) {
      values[5] = data[8] ;
    }
    
    // Data 9  indicates the number of particles with diameter beyond 2.5  um in 0.1 L of air.
    if (lenvalues >= 7) {
      values[6] = data[9] ;
    }
    
    // Data 10 indicates the number of particles with diameter beyond 5.0  um in 0.1 L of air.
    if (lenvalues >= 8) {
      values[7] = data[10] ;
    }
    
    // Data 11 indicates the number of particles with diameter beyond 10.0 um in 0.1 L of air.
    if (lenvalues >= 9) {
      values[8] = data[11] ;
    }
    
    return SD_SUCCESS;
  } else {
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#pms error")));
    return SD_INTERNAL_ERROR;    
  }
}

#if defined (USEGETDATA)
int SensorDriverPMSoneshotSerial::getdata(uint32_t& data,uint16_t& width)
{

  uint32_t values[1];
  width=20;
  const int16_t reference=0;
  
  if (SensorDriverPMSoneshotSerial::get(values,1) == SD_SUCCESS){
    data=(values[0]-reference);// << (sizeof(values[1])-width);
  }else{
    data=0xFFFFFFFF;
    width=0xFFFF;
    return SD_INTERNAL_ERROR;
  }
  return SD_SUCCESS;
  
}
#endif

#if defined(USEAJSON)
aJsonObject* SensorDriverPMSoneshotSerial::getJson()
{
  uint32_t values[9];

  aJsonObject* jsonvalues;
  jsonvalues = aJson.createObject();
  if (SensorDriverPMSoneshotSerial::get(values,9) == SD_SUCCESS){
    if (values[0] != 0xFFFFFFFF){
      aJson.addNumberToObject(jsonvalues, "B15198", (int32_t)values[0]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B15198");
    }

    if (values[1] != 0xFFFFFFFF){
      aJson.addNumberToObject(jsonvalues, "B15195", (int32_t)values[1]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B15195");
    }

    if (values[2] != 0xFFFFFFFF){
      aJson.addNumberToObject(jsonvalues, "B15203", (int32_t)values[2]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B15203");
    }

    if (values[3] != 0xFFFFFFFF){
      aJson.addNumberToObject(jsonvalues, "B49192", (int32_t)values[3]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B49192");
    }
    if (values[4] != 0xFFFFFFFF){
      aJson.addNumberToObject(jsonvalues, "B49193", (int32_t)values[4]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B49193");
    }
    if (values[5] != 0xFFFFFFFF){
      aJson.addNumberToObject(jsonvalues, "B49194", (int32_t)values[5]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B49194");
    }
    if (values[6] != 0xFFFFFFFF){
      aJson.addNumberToObject(jsonvalues, "B49195", (int32_t)values[6]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B49195");
    }
    if (values[7] != 0xFFFFFFFF){
      aJson.addNumberToObject(jsonvalues, "B49196", (int32_t)values[7]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B49196");
    }
    if (values[8] != 0xFFFFFFFF){
      aJson.addNumberToObject(jsonvalues, "B49197", (int32_t)values[8]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B49197");
    }
    
    
  }else{
    aJson.addNullToObject(jsonvalues, "B15198");
    aJson.addNullToObject(jsonvalues, "B15195");
    aJson.addNullToObject(jsonvalues, "B15203");
    aJson.addNullToObject(jsonvalues, "B49192");
    aJson.addNullToObject(jsonvalues, "B49193");
    aJson.addNullToObject(jsonvalues, "B49194");
    aJson.addNullToObject(jsonvalues, "B49195");
    aJson.addNullToObject(jsonvalues, "B49196");
    aJson.addNullToObject(jsonvalues, "B49197");
  }
  return jsonvalues;
}
#endif

#if defined(USEARDUINOJSON)
int SensorDriverPMSoneshotSerial::getJson(char *json_buffer, size_t json_buffer_length)
{
  uint32_t values[9];
  StaticJsonDocument<200> doc;

  if (get(values,9) == SD_SUCCESS){
    if ((uint32_t)values[0] != 0xFFFFFFFF){
      doc["B15198"]= values[0];      
    }else{
      doc["B15198"]=serialized("null");
    }

    if ((uint32_t) values[1] != 0xFFFFFFFF){
      doc["B15195"]= values[1];
    }else{
      doc["B15195"]=serialized("null");
    }

    if ((uint32_t) values[2] != 0xFFFFFFFF){
      doc["B15203"]= values[2];
    }else{
      doc["B15203"]=serialized("null");
    }

    if ((uint32_t) values[3] != 0xFFFFFFFF){
      doc["B49192"]= values[3];
    }else{
      doc["B49192"]=serialized("null");
    }
    if ((uint32_t) values[4] != 0xFFFFFFFF){
      doc["B49193"]= values[4];
    }else{
      doc["B49193"]=serialized("null");
    }
    if ((uint32_t) values[5] != 0xFFFFFFFF){
      doc["B49194"]= values[5];
    }else{
      doc["B49194"]=serialized("null");
    }
    if ((uint32_t) values[6] != 0xFFFFFFFF){
      doc["B49195"]= values[6];
    }else{
      doc["B49195"]=serialized("null");
    }
    if ((uint32_t) values[7] != 0xFFFFFFFF){
      doc["B49196"]= values[7];
    }else{
      doc["B49196"]=serialized("null");
    }
    if ((uint32_t) values[8] != 0xFFFFFFFF){
      doc["B49197"]= values[8];
    }else{
      doc["B49197"]=serialized("null");
    }
    
  }else{
    doc["B15198"]=serialized("null");
    doc["B15195"]=serialized("null");
    doc["B15203"]=serialized("null");
    doc["B49192"]=serialized("null");
    doc["B49193"]=serialized("null");
    doc["B49194"]=serialized("null");
    doc["B49195"]=serialized("null");
    doc["B49196"]=serialized("null");
    doc["B49197"]=serialized("null");
  }

  serializeJson(doc,json_buffer, json_buffer_length);
  return SD_SUCCESS;
}
#endif

//destructor
SensorDriverPMSoneshotSerial::~SensorDriverPMSoneshotSerial(){

  delete _pms;
  //warning: deleting object of polymorphic class type 'SoftwareSerial' which has non-virtual destructor might cause undefined behaviour [-Wdelete-non-virtual-dtor]
  //delete _pmsSerial;
}

#endif



#if defined (SCD_ONESHOT)

SensorDriverSCDoneshot::SensorDriverSCDoneshot()
{
  _scd = new SCD30();
}

int SensorDriverSCDoneshot::setup(const char* driver, const int address, const int node, const char* type)
{

  SensorDriver::setup(driver,address,node,type);
  //bool oneshot=true;

  IF_SDSDEBUG(SDDBGSERIAL.println(F("#try to build SCD")));
  
  _scd->begin(address);  //This will cause readings to occur every two seconds

  /*
    Maximal I2C speed is 100 kHz and the master has to support clock
    stretching. Clock stretching period in write- and read- frames is 12
    ms, however, due to internal calibration processes a maximal clock
    stretching of 150 ms may occur once per day.  For detailed information
    to the I2C protocol, refer to NXP I2C-bus specification 1 . SCD30 does
    not support repeated start condition. Clock stretching is necessary to
    start the microcontroller and might occur before every ACK. I2C master
    clock stretching needs to be implemented according to the NXP
    specification. The boot-up time is < 2 s.
  */
   _scd->sendCommand(COMMAND_SOFT_RESET);
  delay(50);  // ??? not explained in documentation
  if(_scd->beginMeasuring()) { //Start continuous measurements
    IF_SDSDEBUG(SDDBGSERIAL.println(F("# scd beginMeasuring ok")));
    if(_scd->setMeasurementInterval(2)) { //2 seconds between measurements
      IF_SDSDEBUG(SDDBGSERIAL.println(F("# scd setMeasurementInterval ok")));
      if (_scd->setAutoSelfCalibration(true)) { //Enable auto-self-calibration
	IF_SDSDEBUG(SDDBGSERIAL.println(F("# scd setAutoSelfCalibration ok")));

	SCDstarted=false;
	_timing=millis();

	return SD_SUCCESS;
      }
    }
  }

  return SD_INTERNAL_ERROR;

}

int SensorDriverSCDoneshot::prepare(uint32_t& waittime)
{
  
  SCDstarted=true;
  _timing=millis();
  //waittime= 6000ul;

  // clear previous measurements
  _scd->getCO2();
  _scd->getTemperature();
  _scd->getHumidity();
  
  //_scd->beginMeasuring();
    
  waittime= 2500ul;
  return SD_SUCCESS;
}

int SensorDriverSCDoneshot::get(uint32_t values[],size_t lenvalues)
{
  if (millis() - _timing > MAXDELAYFORREAD) return SD_INTERNAL_ERROR;
  if (!SCDstarted)  return SD_INTERNAL_ERROR;
  
  SCDstarted=false;  
  _timing=0;

  if (_scd->dataAvailable()){
  
      // measure
      // get CO2
      if (lenvalues >= 1) {

	// Campo di misura di uno strumento tipco da 0 a 10000 ppm CO₂ Risoluzione 1 ppm CO₂
	//https://www.lenntech.com/calculators/ppm/converter-parts-per-million.htm 1.938   ?????
	// 1,800009 coefficiente di conversione ppm-> mg/m3   (riferito a 25°C e 760 mm Hg)
	// quindi assumioamo 0-0.020 Kg/m3 con risoluzione 0.000002  circa
	// in interi fattore di scala in tabella B 10**6 quindi mg/m3

	values[0] =  _scd->getCO2()* 1.8;
      }
      
      // get temperature
      if (lenvalues >= 2) {
	values[1] = _scd->getTemperature()*100+27315 ;
      }
      
      // get humidity
      if (lenvalues >= 3) {
	values[2] = _scd->getHumidity() ;
      }

      //_scd->sendCommand(COMMAND_STOP_CONTINUOS_MEASUREMENT);
      
      return SD_SUCCESS;
    } else {
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#scd error")));
    return SD_INTERNAL_ERROR;    
  }
}

#if defined (USEGETDATA)
int SensorDriverSCDoneshot::getdata(uint32_t& data,uint16_t& width)
{

  uint32_t values[1];
  width=20;   // todo
  const int16_t reference=0;
  
  if (SensorDriverSCDoneshot::get(values,1) == SD_SUCCESS){
    data=(values[0]-reference);// << (sizeof(values[1])-width);
  }else{
    data=0xFFFFFFFF;
    width=0xFFFF;
    return SD_INTERNAL_ERROR;
  }
  return SD_SUCCESS;
  
}
#endif

#if defined(USEAJSON)
aJsonObject* SensorDriverSCDoneshot::getJson()
{
  uint32_t values[3];

  aJsonObject* jsonvalues;
  jsonvalues = aJson.createObject();
  if (SensorDriverSCDoneshot::get(values,3) == SD_SUCCESS){
    if (values[0] != 0xFFFFFFFF){
      aJson.addNumberToObject(jsonvalues, "B15242", (int32_t)values[0]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B15242");
    }

#if defined(SECONDARYPARAMETER)
    if (values[1] != 0xFFFFFFFF){
      aJson.addNumberToObject(jsonvalues, "B12101", (int32_t)values[1]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B12101");
    }

    if (values[2] != 0xFFFFFFFF){
      aJson.addNumberToObject(jsonvalues, "B13003", (int32_t)values[2]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B13003");
    }
#endif
    
  }else{
    aJson.addNullToObject(jsonvalues, "B15242");
#if defined(SECONDARYPARAMETER)
    aJson.addNullToObject(jsonvalues, "B12101");
    aJson.addNullToObject(jsonvalues, "B13003");
#endif    
  }
  return jsonvalues;
}
#endif

#if defined(USEARDUINOJSON)
int SensorDriverSCDoneshot::getJson(char *json_buffer, size_t json_buffer_length)
{
  uint32_t values[3];
  StaticJsonDocument<200> doc;

  if (get(values,3) == SD_SUCCESS){
    if ((uint32_t)values[0] != 0xFFFFFFFF){
      doc["B15242"]= values[0];      
    }else{
      doc["B15242"]=serialized("null");
    }

#if defined(SECONDARYPARAMETER)
    if ((uint32_t) values[1] != 0xFFFFFFFF){
      doc["B12101"]= values[1];
    }else{
      doc["B12101"]=serialized("null");
    }

    if ((uint32_t) values[2] != 0xFFFFFFFF){
      doc["B13003"]= values[2];
    }else{
      doc["B13003"]=serialized("null");
    }
#endif

  }else{
    doc["B15242"]=serialized("null");
#if defined(SECONDARYPARAMETER)
    doc["B12101"]=serialized("null");
    doc["B13003"]=serialized("null");
#endif
  }

  serializeJson(doc,json_buffer, json_buffer_length);
  return SD_SUCCESS;
}
#endif

//destructor
SensorDriverSCDoneshot::~SensorDriverSCDoneshot(){

  // _scd->sendCommand(COMMAND_STOP_CONTINUOS_MEASUREMENT);

  delete _scd;
  //warning: deleting object of polymorphic class type 'SoftwareSerial' which has non-virtual destructor might cause undefined behaviour [-Wdelete-non-virtual-dtor]
  //delete _pmsSerial;
}
#endif



#if defined (SHTDRIVER)

SensorDriverSHT85::SensorDriverSHT85()
{
  _sht = new SHTI2cSensor();
}


int SensorDriverSHT85::setup(const char* driver, const int address, const int node, const char* type)
{

  SensorDriver::setup(driver,address,node,type);

  //_sht = new SHTI2cSensor();

  IF_SDSDEBUG(SDDBGSERIAL.println(F("#try to build SHT85")));

  //  WARNING !!!!! address is not used; sensirion have only one address
  _sht->softReset();
  delay(10);
  
  if(_sht->clearStatusRegister() != true) //Start continuous measurements
    {
      return SD_INTERNAL_ERROR;
    }

  SHTstarted=false;
  _timing=millis();

  return SD_SUCCESS;
}

int SensorDriverSHT85::prepare(uint32_t& waittime)
{
  
  SHTstarted=true;
  _timing=millis();

  if (!_sht->singleShotDataAcquisition())
    {
      waittime= 1ul;
      return SD_INTERNAL_ERROR;
    }
  
  waittime= _sht->mDuration;
  return SD_SUCCESS;
}

int SensorDriverSHT85::get(uint32_t values[],size_t lenvalues)
{

  if (millis() - _timing > MAXDELAYFORREAD)     return SD_INTERNAL_ERROR;

  if (!_sht->getValues()){
    return SD_INTERNAL_ERROR;
  }

  if (!_sht->checkStatus()){
    return SD_INTERNAL_ERROR;
  }

  if (lenvalues >= 1)  values[0] = (uint32_t) round(_sht->getTemperature() * 100. + 27315.) ;
  if (lenvalues >= 2)  values[1] = (uint32_t) round (_sht->getHumidity()) ;
  _timing=0;

  return SD_SUCCESS;

}

#if defined (USEGETDATA)
int SensorDriverSHT85::getdata(uint32_t& data,uint16_t& width)
{
  /*
    scale: The exponent of the  power of 10 by which the value of the element has been multiplied prior to encoding 
    reference value: A number to be subtracted from the element, after scaling (if any), and prior to encoding 
    data width (bits): The number of bits the element requires for representation in data
  */
  
  uint32_t values[2];
  
  if (SensorDriverSHT85::get(values,2) == SD_SUCCESS){
    int16_t reference=22315;  
    data=(values[0]-reference) ;// << (sizeof(values[1])-width);
    width=16;

    reference=0;
    data |=(values[1]-reference)<< (sizeof(values[1])-width);   //   da controllare
    width+=7;


    
  }else{
    data=0xFFFFFFFF;
    width=0xFFFF;
    return SD_INTERNAL_ERROR;
  }
  return SD_SUCCESS;
}
#endif

#if defined(USEAJSON)
aJsonObject* SensorDriverSHT85::getJson()
{
  uint32_t values[2];

  aJsonObject* jsonvalues;
  jsonvalues = aJson.createObject();
  //if (SensorDriverTmp::get2(&humidity,&temperature) == SD_SUCCESS){
  if (SensorDriverSHT85::get(values,2) == SD_SUCCESS){
    aJson.addNumberToObject(jsonvalues, "B12101", (int32_t)values[0]);      

    // if you have a second value add here
    aJson.addNumberToObject(jsonvalues, "B13003", (int32_t)values[1]);      
  }else{
    aJson.addNullToObject(jsonvalues, "B12101");
    // if you have a second value add here
    aJson.addNullToObject(jsonvalues, "B13003");
  }
  return jsonvalues;
}
#endif
#if defined(USEARDUINOJSON)
int SensorDriverSHT85::getJson(char *json_buffer, size_t json_buffer_length)
{
  uint32_t values[2];
  StaticJsonDocument<200> doc;

  if (get(values,2) == SD_SUCCESS){
    if (values[0] >= 0){
      doc["B12101"]= values[0];      
    }else{
      doc["B12101"]=serialized("null");
    }
    // if you have a second value add here
    if (values[1] >= 0){
      doc["B13003"]= values[1];      
    }else{
      doc["B13003"]=serialized("null");
    }
  }else{
    doc["B12101"]=serialized("null");
    // if you have a second value add here
    doc["B13003"]=serialized("null");
  }

  serializeJson(doc,json_buffer, json_buffer_length);
  return SD_SUCCESS;
}
#endif

//destructor
SensorDriverSHT85::~SensorDriverSHT85(){

  // _scd->sendCommand(COMMAND_STOP_CONTINUOS_MEASUREMENT);

  delete _sht;
  //warning: deleting object of polymorphic class type 'SoftwareSerial' which has non-virtual destructor might cause undefined behaviour [-Wdelete-non-virtual-dtor]
  //delete _pmsSerial;
}

#endif

#if defined (SPS_ONESHOT)

/*
void ErrtoMess(uint8_t r)
{
  char buf[80];
  _sps30->GetErrDescription(r, buf, 80);
  LOGE(buf);
}
*/


SensorDriverSPSoneshot::SensorDriverSPSoneshot()
{
  _sps30 = new SPS30();
}


int SensorDriverSPSoneshot::setup(const char* driver, const int address, const int node, const char* type)
{

  SensorDriver::setup(driver,address,node,type);

  IF_SDSDEBUG(SDDBGSERIAL.println(F("#try to build SPS")));
  
  if (_sps30->begin(SP30_COMMS)){
    if (!_sps30->probe()){
      IF_SDSDEBUG(SDDBGSERIAL.println(F("SPS probe error")));
    }
      
    if (!_sps30->reset()){
      IF_SDSDEBUG(SDDBGSERIAL.println(F("SPS reset error")));
    }
    delay(100);

    //try to read serial number
    char buf[32];
    if (_sps30->GetSerialNumber(buf, 32) == SPS30_ERR_OK) {
      IF_SDSDEBUG(SDDBGSERIAL.print(F("#SP30 Serial number : ")));
      if(strlen(buf) > 0) {
	IF_SDSDEBUG(SDDBGSERIAL.println(buf));
      }else {
	IF_SDSDEBUG(SDDBGSERIAL.println(F("SPS cannot get serialnumber")));
      }
    }else{
      IF_SDSDEBUG(SDDBGSERIAL.println(F("SPS GetSerialNumber error")));      
    }

#ifdef ONESHOT_SWITCHOFF	
    SPSstarted=false;
    _timing=millis();

    return SD_SUCCESS;
#else
    delay(10);
    if(_sps30->start()){
      SPSstarted=true;
      _timing=millis();
	
      return SD_SUCCESS;
    }
#endif
  }

  return SD_INTERNAL_ERROR;
}

int SensorDriverSPSoneshot::prepare(uint32_t& waittime)
{
  
#ifdef ONESHOT_SWITCHOFF
  if(!_sps30->start()){
    return SD_INTERNAL_ERROR;
  }
  waittime= 10000ul;
#else
  waittime= 1000ul;
#endif

  SPSstarted=true;
  _timing=millis();
  return SD_SUCCESS;
}

int SensorDriverSPSoneshot::get(uint32_t values[],size_t lenvalues)
{
  // measure
  struct sps_values val;

  if (millis() - _timing > MAXDELAYFORREAD) return SD_INTERNAL_ERROR;
  if (!SPSstarted)  return SD_INTERNAL_ERROR;

  SPSstarted=false;  
  _timing=0;

  for (size_t i=0; i<lenvalues; i++)
    {
      values[i]=0xFFFFFFFF;
    }
  
  // get data
  // data might not have been ready
  if (_sps30->GetValues(&val) != SPS30_ERR_OK){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#sps getvalues error")));
#ifdef ONESHOT_SWITCHOFF
    _sps30->stop();
#endif
    return SD_INTERNAL_ERROR;    
  }
#ifdef ONESHOT_SWITCHOFF
  if (!_sps30->stop()){
    IF_SDSDEBUG(SDDBGSERIAL.println(F("#sps stop error")));
    return SD_INTERNAL_ERROR;    
  }
#endif

  // get pm1
  if (lenvalues >= 1) {
    values[0] = round(val.MassPM1*10.) ;
  }

  // get pm2
  if (lenvalues >= 2) {
    values[1] = round(val.MassPM2*10.) ;
  }
  
  // get pm4
  if (lenvalues >= 3) {
    values[2] = round(val.MassPM4*10.) ;
  }
  
  // get pm10
  if (lenvalues >= 4) {
    values[3] = round(val.MassPM10*10.) ;
  }
    
  if ((SP30_COMMS == I2C_COMMS) && (_sps30->I2C_expect() == 4)){
    IF_SDSDEBUG(SDDBGSERIAL.println(F(" !!! Due to I2C buffersize only the SPS30 MASS concentration is available !!! \n")));
  }else{
  
    // number of particles with diameter 0.3 to 0.5 um in 0.1 L of air.
    if (lenvalues >= 5) {
      values[4] = round(val.NumPM0 *10.);
    }

    // number of particles with diameter 0.5 to 1.0  um in 0.1 L of air.
    if (lenvalues >= 6) {
      values[5] = round((val.NumPM1-val.NumPM0)*100.) ;
    }
  
    // number of particles with diameter 1.0 to 2.5 um in 0.1 L of air.
    if (lenvalues >= 7) {
      values[6] = round((val.NumPM2-val.NumPM1)*1000.) ;
    }
  
    // number of particles with diameter 2.5 to 5.0 (4.0) um in 0.1 L of air.
    if (lenvalues >= 8) {
      values[7] = round((val.NumPM4-val.NumPM2)*10000.) ;
    }
  
    // number of particles with diameter 5.0 to 10 um in 0.1 L of air.
    if (lenvalues >= 9) {
      values[8] = round((val.NumPM10-val.NumPM4)*100000.) ;
    }

    /*
    // get particulte size
    if (lenvalues >= 10) {
    values[9] = val.PartSize ;
    }
    */
  }

  return SD_SUCCESS;
}

#if defined (USEGETDATA)
int SensorDriverSPSoneshot::getdata(uint32_t& data,uint16_t& width)
{

  uint32_t values[2];
  width=20;
  const int16_t reference=0;
  
  if (SensorDriverSPSoneshot::get(values,2) == SD_SUCCESS){
    data=(values[1]-reference);// << (sizeof(values[1])-width);
  }else{
    data=0xFFFFFFFF;
    width=0xFFFF;
    return SD_INTERNAL_ERROR;
  }
  return SD_SUCCESS;
  
}
#endif

#if defined(USEAJSON)
aJsonObject* SensorDriverSPSoneshot::getJson()
{
  uint32_t values[9];

  aJsonObject* jsonvalues;
  jsonvalues = aJson.createObject();
  if (SensorDriverSPSoneshot::get(values,9) == SD_SUCCESS){
    if (values[0] != 0xFFFFFFFF){
      aJson.addNumberToObject(jsonvalues, "B15203", (int32_t)values[0]);    //PM1  
    }else{
      aJson.addNullToObject(jsonvalues, "B15203");
    }
    if (values[1] != 0xFFFFFFFF){
      aJson.addNumberToObject(jsonvalues, "B15198", (int32_t)values[1]);   //PM2.5
    }else{
      aJson.addNullToObject(jsonvalues, "B15198");
    }
    if (values[2] != 0xFFFFFFFF){
      aJson.addNumberToObject(jsonvalues, "B15202", (int32_t)values[2]);   //PM5 (4)
    }else{
      aJson.addNullToObject(jsonvalues, "B15202");
    }
    if (values[3] != 0xFFFFFFFF){
      aJson.addNumberToObject(jsonvalues, "B15195", (int32_t)values[3]);   //PM10
    }else{
      aJson.addNullToObject(jsonvalues, "B15195");
    }
    if (values[4] != 0xFFFFFFFF){
      aJson.addNumberToObject(jsonvalues, "B49193", (int32_t)values[4]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B49193");
    }
    if (values[5] != 0xFFFFFFFF){
      aJson.addNumberToObject(jsonvalues, "B49194", (int32_t)values[5]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B49194");
    }
    if (values[6] != 0xFFFFFFFF){
      aJson.addNumberToObject(jsonvalues, "B49195", (int32_t)values[6]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B49195");
    }
    if (values[7] != 0xFFFFFFFF){
      aJson.addNumberToObject(jsonvalues, "B49196", (int32_t)values[7]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B49196");
    }
    if (values[8] != 0xFFFFFFFF){
      aJson.addNumberToObject(jsonvalues, "B49197", (int32_t)values[8]);      
    }else{
      aJson.addNullToObject(jsonvalues, "B49197");
    }
    
  }else{
    
    aJson.addNullToObject(jsonvalues, "B15203");
    aJson.addNullToObject(jsonvalues, "B15198");
    aJson.addNullToObject(jsonvalues, "B15202");
    aJson.addNullToObject(jsonvalues, "B15195");
    aJson.addNullToObject(jsonvalues, "B49193");
    aJson.addNullToObject(jsonvalues, "B49194");
    aJson.addNullToObject(jsonvalues, "B49195");
    aJson.addNullToObject(jsonvalues, "B49196");
    aJson.addNullToObject(jsonvalues, "B49197");
  }
  return jsonvalues;
}
#endif

#if defined(USEARDUINOJSON)
int SensorDriverSPSoneshot::getJson(char *json_buffer, size_t json_buffer_length)
{
  uint32_t values[9];
  StaticJsonDocument<200> doc;

  if (SensorDriverSPSoneshot::get(values,9) == SD_SUCCESS){

    if ((uint32_t)values[0] != 0xFFFFFFFF){
      doc["B15203"]= values[0];      
    }else{
      doc["B15203"]=serialized("null");
    }
    if ((uint32_t) values[1] != 0xFFFFFFFF){
      doc["B15198"]= values[1];
    }else{
      doc["B15198"]=serialized("null");
    }
    if ((uint32_t) values[2] != 0xFFFFFFFF){
      doc["B15202"]= values[2];
    }else{
      doc["B15202"]=serialized("null");
    }
    if ((uint32_t) values[3] != 0xFFFFFFFF){
      doc["B15195"]= values[3];
    }else{
      doc["B15195"]=serialized("null");
    }    
    if ((uint32_t) values[4] != 0xFFFFFFFF){
      doc["B49193"]= values[4];
    }else{
      doc["B49193"]=serialized("null");
    }
    if ((uint32_t) values[5] != 0xFFFFFFFF){
      doc["B49194"]= values[5];
    }else{
      doc["B49194"]=serialized("null");
    }
    if ((uint32_t) values[6] != 0xFFFFFFFF){
      doc["B49195"]= values[6];
    }else{
      doc["B49195"]=serialized("null");
    }
    if ((uint32_t) values[7] != 0xFFFFFFFF){
      doc["B49196"]= values[7];
    }else{
      doc["B49196"]=serialized("null");
    }
    if ((uint32_t) values[8] != 0xFFFFFFFF){
      doc["B49197"]= values[8];
    }else{
      doc["B49197"]=serialized("null");
    }
  }else{
    doc["B15203"]=serialized("null");
    doc["B15198"]=serialized("null");
    doc["B15202"]=serialized("null");
    doc["B15195"]=serialized("null");
    doc["B49193"]=serialized("null");
    doc["B49194"]=serialized("null");
    doc["B49195"]=serialized("null");
    doc["B49196"]=serialized("null");
    doc["B49197"]=serialized("null");
  }

  serializeJson(doc,json_buffer, json_buffer_length);
  return SD_SUCCESS;
}
#endif

//destructor
SensorDriverSPSoneshot::~SensorDriverSPSoneshot(){

  delete _sps30;
  //warning: deleting object of polymorphic class type 'SoftwareSerial' which has non-virtual destructor might cause undefined behaviour [-Wdelete-non-virtual-dtor]
}

#endif

