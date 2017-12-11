#ifndef arduinoJsonRPC_h
#define arduinoJsonRPC_h

#include "Arduino.h"
#include <ArduinoJson.h>
#include "JsonRPCerror.h"


struct Mapping
{
    String name;
  int (*callback)(JsonObject&,JsonObject&);
};

struct FuncMap
{
    Mapping* mappings;
    unsigned int capacity;
    unsigned int used;
};

class JsonRPC
{
    public:
  JsonRPC(int capacity, bool radio=false);
  void registerMethod(String methodname, int(*callback)(JsonObject&,JsonObject&));
  int processMessage(JsonObject& params);
    private:
	FuncMap* mymap;
	bool myradio;
};

#endif
