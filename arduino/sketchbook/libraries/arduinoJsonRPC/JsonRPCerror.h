#ifndef JsonRPCerror_h
#define JsonRPCerror_h

enum _error
{
    E_SUCCESS = 0,
    E_BUSY = -1,
    E_PARSE_ERROR=-32700,
    E_INVALID_REQUEST=-32600,
    E_METHOD_NOT_FOUND=-32601,
    E_INVALID_PARAMS=-32602,
    E_INTERNAL_ERROR=-32603
    //E_SERVER_ERROR// -32000 to -32099 	Server error 	Reserved for implementation-defined server-errors.
};

/* type to provide in your API */
typedef _error error_t;

const __FlashStringHelper*  jsstrerror (int errnum);


#endif
