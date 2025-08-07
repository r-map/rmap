#include "arduinoJsonRPC.h"

Mapping::Mapping() : callback(nullptr) {
   name[0] = '\0';
}

FuncMap::FuncMap() : used(0) {}

JsonRPC::JsonRPC(bool my_radio) {
   radio = my_radio;
   jrpc_state = JRPC_INIT;
}

void JsonRPC::registerMethod(const char* methodName, int (*callback)(JsonObject, JsonObject)) {
   // only write keyvalue pair if we allocated enough memory for it
   if (mymap.used < JRPC_MAXRPC) {
      Mapping& mapping = mymap.mappings[mymap.used];
      strncpy(mapping.name, methodName, sizeof(mapping.name)-1);
      mapping.name[sizeof(mapping.name)-1]='\0';
      mapping.callback = callback;
      mymap.used++;
   }
}

int JsonRPC::processMessage() {

   if (!doc.containsKey(radio ? F("i") : F("id")) || !doc.containsKey(radio ? F("m") : F("method"))) {
     // id or method are missed
     doc.to<JsonObject>(); // clean the doc
     return E_PARSE_ERROR;
   }
   const char* method = doc[radio ? F("m") : F("method")].as<const char*>();
   if (method == NULL){
     doc.to<JsonObject>(); // clean the doc
      return E_PARSE_ERROR;
   }

   int ret_status = E_METHOD_NOT_FOUND;

   for (unsigned int i=0; i<mymap.used; i++) {
     mapping = &mymap.mappings[i];
     if (strcmp(method, mapping->name) == 0) {
       doc.remove(radio? F("m") : F("method"));
       return E_BUSY;
     }
   }

   if (ret_status != E_BUSY && ret_status != E_SUCCESS) {
     //jsonrpc response with error and message
     
     doc.remove(radio ? F("m") : F("method"));
     doc.remove(radio ? F("p") : F("params"));
     doc.remove(radio ? F("r") : F("result"));
     JsonObject error = doc.createNestedObject(radio ? "e" : "error");
     error[radio ? F("c") : F("code")] = ret_status;
     
     if (radio == 0) {
       error[F("message")]= jsstrerror(ret_status);
     }
   }
   return ret_status;
}

void JsonRPC::parseCharpointer(bool *is_active, char *rpcin, const size_t rpcin_len, char *rpcout, const size_t rpcout_len )
{
  int status;
  
  switch (jrpc_state) {
  case JRPC_INIT:
      *is_active = true;
      jrpc_state = JRPC_AVAILABLE;
    break;

  case JRPC_AVAILABLE:
    {
      DeserializationError error =  deserializeJson(doc,rpcin,rpcin_len);
      if (error) {
	jrpc_state = JRPC_END;
      }else{
	status = processMessage();
	if (status != E_BUSY && status != E_SUCCESS) {
	  serializeJson(doc,rpcout,rpcout_len);
	  jrpc_state = JRPC_END;
	}
	else {
	  jrpc_state = JRPC_PROCESS;
	}
      }
    }
    
    break;
    
  case JRPC_PROCESS:
    
    status = callback();

    if (status != E_BUSY) {
      jrpc_state = JRPC_END;
      serializeJson(doc,rpcout,rpcout_len);
    }
    break;
    
  case JRPC_END:

    jrpc_state = JRPC_INIT;
    *is_active = false;
    break;
  }
}


void JsonRPC::parseStream(bool *is_active, Stream *stream, const uint32_t timeout) {

  int status;
  
  switch (jrpc_state) {
  case JRPC_INIT:
    stream->setTimeout(timeout);
    if (stream->available()) {
      *is_active = true;
      jrpc_state = JRPC_AVAILABLE;
    } else {
      *is_active = false;
    }
    break;

  case JRPC_AVAILABLE:
    {
      stream->setTimeout(timeout);
      DeserializationError error =  deserializeJson(doc,*stream);
      if (error) {
	while (stream->available()) stream->read();  //clean stream
	jrpc_state = JRPC_END;
      }else{
	status = processMessage();
	if (status != E_BUSY && status != E_SUCCESS) {
	  serializeJson(doc,*stream);
	  stream->print("\n");
	  jrpc_state = JRPC_END;
	}
	else {
	  jrpc_state = JRPC_PROCESS;
	}
      }
    }
    
    break;
    
  case JRPC_PROCESS:
    
    status = callback();

    if (status != E_BUSY) {
      jrpc_state = JRPC_END;
      serializeJson(doc,*stream);
      stream->print("\n");
    }
    break;
    
  case JRPC_END:

    jrpc_state = JRPC_INIT;
    *is_active = false;
    break;
  }
}

int JsonRPC::callback() {
   JsonObject params = doc[radio ? F("p") : F("params")];
   JsonObject result = doc.createNestedObject(radio ? F("r") : F("result"));

   int ret_status = mapping->callback(params, result);

   doc.remove(radio? F("p") : F("params"));
   
   if (ret_status != E_BUSY && ret_status != E_SUCCESS) {
      doc.remove(radio ? F("r") : F("result"));
      JsonObject error = doc.createNestedObject(radio ? "e" : "error");
      error[radio ? F("c") : F("code")] = ret_status;

      if (radio == 0) {
         error[F("message")]= jsstrerror(ret_status);
      }
   }

   return ret_status;
}

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
