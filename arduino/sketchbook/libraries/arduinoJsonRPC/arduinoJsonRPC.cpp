#include "Arduino.h"
#include "arduinoJsonRPC.h"

Mapping::Mapping() : callback(nullptr) {
   name[0] = '\0';
}

FuncMap::FuncMap() : used(0) {}

JsonRPC::JsonRPC(bool my_radio) {
   radio = my_radio;
   is_stream_ready = true;
   do_stream_read = false;
}

void JsonRPC::registerMethod(const char* methodName, int (*callback)(JsonObject &, JsonObject &)) {
   // only write keyvalue pair if we allocated enough memory for it
   if (mymap.used < JRPC_MAXRPC) {
      Mapping& mapping = mymap.mappings[mymap.used];
      strncpy(mapping.name, methodName, sizeof(mapping.name)-1);
      mapping.name[sizeof(mapping.name)-1]='\0';
      mapping.callback = callback;
      mymap.used++;
   }
}

int JsonRPC::processMessage(JsonObject &msg) {
   int status = E_SUCCESS;

   if (!msg.containsKey(radio ? F("i") : F("id"))) {
      status = E_PARSE_ERROR;
   }

   const char* method = msg[radio ? F("m") : F("method")];
   if (method == NULL){
      status = E_PARSE_ERROR;
   }

   if (status == E_SUCCESS) {
      status = E_METHOD_NOT_FOUND;
      for (unsigned int i=0; i<mymap.used; i++) {
         mapping = &mymap.mappings[i];
         if (strcmp(method, mapping->name) == 0) {
            msg.remove(radio? F("m") : F("method"));

            #if (JRPC_MODE == JRPC_CLASSIC_MODE)
               JsonObject &params = msg[radio ? F("p") : F("params")];
               msg.remove(radio? F("p") : F("params"));
               JsonObject &result = msg.createNestedObject(radio ? F("r") : F("result"));
               status = mapping->callback(params,result);

            #elif (JRPC_MODE == JRPC_NON_BLOCKING_MODE)
               memset(input_buffer, 0, JRPC_BUFFER_LENGTH);
               msg.printTo(input_buffer, JRPC_BUFFER_LENGTH);
               status = E_BUSY;

            #endif
            break;
         }
      }
   }

   if (status != E_BUSY && status != E_SUCCESS) {
      msg.remove(radio ? F("r") : F("result"));
      JsonObject &error = msg.createNestedObject(radio ? "e" : "error");
      error[radio ? F("c") : F("code")] = status;

      if (radio == 0) {
         error[F("message")]= jsstrerror(status);
      }
   }

   return status;
}

#if (JRPC_MODE == JRPC_NON_BLOCKING_MODE)
int JsonRPC::parseStream(bool *is_active, Stream *stream, uint32_t timeout) {
   static int status;

   switch (jrpc_state) {
      case JRPC_INIT:
         if (is_stream_ready) {
            status = E_BUSY;
            stream->flush();
            stream->setTimeout(timeout);
            while (stream->available()) {
               stream->read();
            }
            jrpc_state = JRPC_AVAILABLE;
         }
      break;

      case JRPC_AVAILABLE:
         noInterrupts();
         if (stream->available() && is_stream_ready) {
            is_stream_ready = false;
            do_stream_read = true;
            *is_active = true;
         }
         else {
            *is_active = false;
         }
         interrupts();

         if (do_stream_read) {
            StaticJsonBuffer<JRPC_BUFFER_LENGTH> jsonBuffer;
            JsonObject &msg = jsonBuffer.parse(*stream);
            if (msg.success()) {
               status = processMessage(msg);
               if (status != E_BUSY && status != E_SUCCESS) {
                  msg.printTo(*stream);
                  jrpc_state = JRPC_END;
               }
               else {
                  jrpc_state = JRPC_PROCESS;
               }
            }
            noInterrupts();
            do_stream_read = false;
            interrupts();
         }
      break;

      case JRPC_PROCESS:
         #if (JRPC_MODE == JRPC_CLASSIC_MODE)
         jrpc_state = JRPC_END;
         #elif (JRPC_MODE == JRPC_NON_BLOCKING_MODE)
         status = callback(stream);

         if (status != E_BUSY) {
            noInterrupts();
            is_stream_ready = true;
            interrupts();
            jrpc_state = JRPC_END;
         }
         #endif
      break;

      case JRPC_END:
         jrpc_state = JRPC_INIT;
      break;
   }

   return status;
}

int JsonRPC::callback(Stream *stream) {
   StaticJsonBuffer<JRPC_BUFFER_LENGTH> jsonBuffer;
   JsonObject &msg = jsonBuffer.parseObject((const char*)input_buffer);
   JsonObject &params = msg[radio ? F("p") : F("params")];
   msg.remove(radio? F("p") : F("params"));
   JsonObject &result = msg.createNestedObject(radio ? F("r") : F("result"));

   int status = mapping->callback(params, result);

   if (status != E_BUSY && status != E_SUCCESS) {
      msg.remove(radio ? F("r") : F("result"));
      JsonObject &error = msg.createNestedObject(radio ? "e" : "error");
      error[radio ? F("c") : F("code")] = status;

      if (radio == 0) {
         error[F("message")]= jsstrerror(status);
      }
   }

   if (status != E_BUSY) {
      msg.printTo(*stream);
   }

   return status;
}
#endif

const __FlashStringHelper* jsstrerror (int errnum) {
   /* use this to provide a perror style method to help consumers out */
   struct _errordesc {
      int  code;
      const __FlashStringHelper* message;
   } errordesc[] = {
      { E_SUCCESS, F("No error") },
      { E_PARSE_ERROR, F("Parse error") },            // Invalid JSON was received by the server. An error occurred on the server while parsing the JSON text.
      { E_INVALID_REQUEST, F("Invalid Request") }, 	// The JSON sent is not a valid Request object.
      { E_METHOD_NOT_FOUND, F("Method not found") }, 	// The method does not exist / is not available.
      { E_INVALID_PARAMS, F("Invalid param") },       // Invalid method parameter(s).
      { E_INTERNAL_ERROR, F("Internal error") },      // Internal JSON-RPC error.
      { E_BUSY, F("Busy") }
      // -32000 to -32099 	Server error 	Reserved for implementation-defined server-errors.
   };

   #define N_ELEMENTS(array) (sizeof(array)/sizeof((array)[0]))

   for (uint8_t i=0; i<=N_ELEMENTS(errordesc); i++) {
      if ( errordesc[i].code == errnum ) {
         return errordesc[i].message;
      }
   }
   return F("");
}
