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

#define JRPC_CLASSIC_MODE           (1)
#define JRPC_NON_BLOCKING_MODE      (2)

#define JRPC_MODE                   (JRPC_NON_BLOCKING_MODE)

#if (JRPC_MODE == JRPC_NON_BLOCKING_MODE)
#define JRPC_BUFFER_LENGTH          (384)
#define JRPC_DEFAULT_TIMEOUT_MS     (5)

typedef enum {
   JRPC_INIT,
   JRPC_AVAILABLE,
   JRPC_PROCESS,
   JRPC_END
} jrpc_state_t;
#endif

struct Mapping {
   char name[JRPC_MAXRPCNAMELEN];
   int (*callback)(JsonObject&,JsonObject&);

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

   #if (JRPC_MODE == JRPC_NON_BLOCKING_MODE)
   int parseStream(bool *is_active, Stream *stream, uint32_t timeout = JRPC_DEFAULT_TIMEOUT_MS);
   int callback(Stream *stream);
   char input_buffer[JRPC_BUFFER_LENGTH];
   #endif

   void registerMethod(const char* methodName, int (*callback)(JsonObject &, JsonObject &));
   int processMessage(JsonObject &msg);

private:
   FuncMap mymap;
   bool radio;
   Mapping *mapping;

   #if (JRPC_MODE == JRPC_NON_BLOCKING_MODE)
   jrpc_state_t jrpc_state;
   bool is_stream_ready;
   bool do_stream_read;
   #endif
};

#endif
