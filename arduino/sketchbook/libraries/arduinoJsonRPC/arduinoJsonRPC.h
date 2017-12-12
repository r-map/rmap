#ifndef arduinoJsonRPC_h
#define arduinoJsonRPC_h

#include "Arduino.h"
#include <ArduinoJson.h>
#include "JsonRPCerror.h"


struct Mapping
{
    char name[24];
    int (*callback)(JsonObject&,JsonObject&);
};

struct FuncMap
{
    Mapping mappings[5];
    unsigned int used;
};

class JsonRPC
{
    public:
  JsonRPC(bool radio=false);
  void registerMethod(String methodname, int(*callback)(JsonObject&,JsonObject&));
  int processMessage(JsonObject& params);
    private:
	FuncMap mymap;
	bool myradio;
};

#endif
