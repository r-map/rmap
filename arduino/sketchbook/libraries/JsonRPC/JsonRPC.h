#ifndef JsonRPC_h
#define JsonRPC_h

#include "Arduino.h"
#include "aJSON.h"
#include "JsonRPCerror.h"


struct Mapping
{
    String name;
    int (*callback)(aJsonObject*);
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
	void registerMethod(String methodname, int(*callback)(aJsonObject*));
	int processMessage(aJsonObject *msg);
    private:
	FuncMap* mymap;
	bool myradio;
};

#endif
