#include "Arduino.h"
#include "aJSON.h"
#include "JsonRPC.h"

JsonRPC::JsonRPC(int capacity,bool radio)
{
    myradio=radio;
    mymap = (FuncMap *)malloc(sizeof(FuncMap));
    mymap->capacity = capacity;
    mymap->used = 0;
    mymap->mappings = (Mapping*)malloc(capacity * sizeof(Mapping));
    memset(mymap->mappings, 0, capacity * sizeof(Mapping));
}

void JsonRPC::registerMethod(String methodName, int(*callback)(aJsonObject*))
{
    // only write keyvalue pair if we allocated enough memory for it
    if (mymap->used < mymap->capacity)
    {
	Mapping* mapping = &(mymap->mappings[mymap->used++]);
	mapping->name = methodName;
	mapping->callback = callback;
    }
}

int JsonRPC::processMessage(aJsonObject *msg)
{
  aJsonObject* method = aJson.getObjectItem(msg, myradio? "m" : "method");
    if (!method)
    {
	// not a valid Json-RPC message
        Serial.flush();
        return E_PARSE_ERROR;
    }
    
    aJsonObject* params = aJson.getObjectItem(msg, myradio? "p" : "params");
    if (!params)
    {
	Serial.flush();
	return E_PARSE_ERROR;
    }
    
    String methodName = method->valuestring;
    for (int i=0; i<mymap->used; i++)
    {
        Mapping* mapping = &(mymap->mappings[i]);
        if (methodName.equals(mapping->name))
        {
	    return mapping->callback(params);
	}
    }

    return E_METHOD_NOT_FOUND;

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
