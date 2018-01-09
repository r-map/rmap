#ifndef arduinoJsonRPC_h
#define arduinoJsonRPC_h

#include "Arduino.h"
#include <ArduinoJson.h>
#include "JsonRPCerror.h"

#ifndef JRPC_MAXRPC
// initialize an instance of the JsonRPC library for registering 
// JRPC_MAXRPC local method
#define JRPC_MAXRPC 5
#endif
#ifndef JRPC_MAXRPCNAMELEN
// define the max len in char for rpc name
#define JRPC_MAXRPCNAMELEN 10
#endif

struct Mapping
{
  char name[JRPC_MAXRPCNAMELEN];
  int (*callback)(JsonObject&,JsonObject&);

public:
    Mapping();
};

struct FuncMap
{
    Mapping mappings[JRPC_MAXRPC];
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
