#ifndef arduinoJsonRPC_h
#define arduinoJsonRPC_h

#include "Arduino.h"
#include <ArduinoJson.h>
#include "JsonRPCerror.h"

#define MAXRPC 5
#define MAXRPCNAMELEN 24

struct Mapping
{
  char name[MAXRPCNAMELEN];
  int (*callback)(JsonObject&,JsonObject&);

public:
    Mapping();
};

struct FuncMap
{
    Mapping mappings[MAXRPC];
    unsigned int used;
public:
    FuncMap();
};

class JsonRPC
{
    public:
  JsonRPC(bool radio=false);
  void registerMethod(const char* methodName, int(*callback)(JsonObject&,JsonObject&));
  int processMessage(JsonObject& params);
    private:
	FuncMap mymap;
	bool myradio;
};

#endif
