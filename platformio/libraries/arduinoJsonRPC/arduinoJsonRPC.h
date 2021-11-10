#ifndef arduinoJsonRPC_h
#define arduinoJsonRPC_h

#include "Arduino.h"
#include <ArduinoJson.h>
#include "JsonRPCerror.h"

#ifndef JRPC_MAXRPC
// initialize an instance of the JsonRPC library for registering
// JRPC_MAXRPC local method
#define JRPC_MAXRPC                 (5)
#endif
#ifndef JRPC_MAXRPCNAMELEN
// define the max len in char for rpc name
#define JRPC_MAXRPCNAMELEN          (15)
#endif

#define JRPC_DOCUMENT_SIZE          (300)
#define JRPC_DEFAULT_TIMEOUT_MS     (5)

typedef enum {
   JRPC_INIT,
   JRPC_AVAILABLE,
   JRPC_PROCESS,
   JRPC_END
} jrpc_state_t;

struct Mapping {
   char name[JRPC_MAXRPCNAMELEN];
   int (*callback)(JsonObject,JsonObject);

public:
   Mapping();
};

struct FuncMap {
   Mapping mappings[JRPC_MAXRPC];
   unsigned int used;
public:
   FuncMap();
};

class JsonRPC {
public:
   JsonRPC(bool my_radio = false);
   void parseStream(bool *is_active, Stream *stream, const uint32_t timeout = JRPC_DEFAULT_TIMEOUT_MS);
   void parseCharpointer(bool *is_active, char *rpcin, const size_t rpcin_len, char *rpcout, const size_t rpcout_len );
   int callback();
   void registerMethod(const char* methodName, int (*callback)(JsonObject, JsonObject));
   int processMessage();

private:
   FuncMap mymap;
   bool radio;
   Mapping *mapping;
   jrpc_state_t jrpc_state;
   StaticJsonDocument<JRPC_DOCUMENT_SIZE> doc;

};

#endif
