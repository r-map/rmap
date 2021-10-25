#include "frtosSensorDriverb.h"


frtosSensorDriver* frtosSensorDriver::create(const char* mydriver,const char* mytype, MutexStandard& mutex) {
 return new frtosSensorDriver(mydriver,mytype,mutex);  
}

frtosSensorDriver::frtosSensorDriver(const char* mydriver,const char* mytype, MutexStandard& mutex) {
  _sd=SensorDriver::create(mydriver,mytype);
  _mutex=mutex;
  driver=mydriver;
  type=mytype;
}

frtosSensorDriver::~frtosSensorDriver() {
  delete _sd;
}


int frtosSensorDriver::setup(const char* mydriver, const int myaddress, const int mynode, const char* mytype)
{
  LockGuard guard(_mutex);
  return _sd->setup(mydriver, myaddress, mynode, mytype);
}

int frtosSensorDriver::prepare(uint32_t& waittime)
{
  LockGuard guard(_mutex);
  return _sd->prepare(waittime);
}

int frtosSensorDriver::get(uint32_t values[],size_t lenvalues)
{
  LockGuard guard(_mutex);
  return _sd->get(values,lenvalues);
}

#if defined (USEGETDATA)
int frtosSensorDriver::getdata(uint32_t& data,uint16_t& width)
{
  LockGuard guard(_mutex);
  return _sd->getdata(data,width);

}
#endif

#if defined(USEAJSON)
aJsonObject* frtosSensorDriver::getJson()
{
  LockGuard guard(_mutex);
  return _sd->getJson();
}
#endif

#if defined(USEARDUINOJSON)
int frtosSensorDriver::getJson(char *json_buffer, size_t json_buffer_length)
{
  LockGuard guard(_mutex);
  return _sd->getJson(json_buffer,json_buffer_length);
}
#endif
