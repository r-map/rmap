#include "Arduino.h"
#include "arduinoJsonRPC.h"

Mapping::Mapping() : callback(nullptr) {
    name[0] = '\0';
}

FuncMap::FuncMap() : used(0) {}

JsonRPC::JsonRPC(bool radio) : myradio(radio) {}

void JsonRPC::registerMethod(const char* methodName, int(*callback)(JsonObject&,JsonObject&))
{
    // only write keyvalue pair if we allocated enough memory for it
    if (mymap.used < 5)
    {
        Mapping& m = &mymap.mappings[mymap.used];
        strcpy(m.name, methodName);
        mapping.callback = callback;
        mymap.used++;
    }
}

int JsonRPC::processMessage(JsonObject& msg) {

  int status = E_SUCCESS;
  if (myradio == 0){
    if (!msg.containsKey("jsonrpc") || !msg.containsKey("id") || !msg.containsKey("method")){
      status = E_PARSE_ERROR;
    }
  }else{
    if (!msg.containsKey("i") || !msg.containsKey("m")){
      status = E_PARSE_ERROR;
    }
  }

  if (status == E_SUCCESS){
    status=  E_METHOD_NOT_FOUND;
    JsonObject& params = msg[myradio? "p" : "params"];
    
    const char* method = msg[myradio? "m" : "method"];
      
    for (int i=0; i<mymap.used; i++) {
        Mapping& mapping = &mymap.mappings[i];
      if (strcmp(method, mapping.name)==0) {	      
	JsonObject& result = msg.createNestedObject(myradio? "r" : "result");
	status = mapping.callback(params,result);	
      }
    }	
  }

  msg.remove(myradio? "m" : "method");
  msg.remove(myradio? "p" : "params");
  if (! (status == E_SUCCESS)){
    msg.remove(myradio? "r" : "result");
    JsonObject& error = msg.createNestedObject(myradio? "e" : "error");
    error[myradio? "c" : "code"]= status;
    if (myradio == 0) error["message"]= strerror(status);   
  }
  return status;
}

char * strerror (int errnum)
{

/* use this to provide a perror style method to help consumers out */
struct _errordesc {
    int  code;
    char *message;
} errordesc[] = {
  { E_SUCCESS, "No error" },
  {E_PARSE_ERROR     ,"Parse error"       },    //Invalid JSON was received by the server. An error occurred on the server while parsing the JSON text.  
  {E_INVALID_REQUEST ,"Invalid Request"   }, 	//The JSON sent is not a valid Request object.							       
  {E_METHOD_NOT_FOUND,"Method not found"  }, 	//The method does not exist / is not available.							       
  {E_INVALID_PARAMS  ,"Invalid param"     },    //Invalid method parameter(s).									       
  {E_INTERNAL_ERROR  ,"Internal error"    }     //Internal JSON-RPC error.                                                                               
// -32000 to -32099 	Server error 	Reserved for implementation-defined server-errors.
};


#define N_ELEMENTS(array) (sizeof(array)/sizeof((array)[0])) 
  
  for(int i=0; i<=N_ELEMENTS(errordesc); i++) {
    if ( errordesc[i].code == errnum )
      {   
	return errordesc[i].message;
      }
  }
  return "";
}
