#include "arduinoJsonRPC.h"

Mapping::Mapping() : callback(nullptr)
{
  name[0] = '\0';
}

FuncMap::FuncMap() : used(0) {}

JsonRPC::JsonRPC()
{
  init();
}

void JsonRPC::init()
{
  jrpc_state = JRPC_INIT;
}

void JsonRPC::registerMethod(const char *methodName, int (*callback)(JsonObject, JsonObject))
{
  // only write keyvalue pair if we allocated enough memory for it
  if (mymap.used < JRPC_MAXRPC)
  {
    Mapping &mapping = mymap.mappings[mymap.used];
    strncpy(mapping.name, methodName, sizeof(mapping.name) - 1);
    mapping.name[sizeof(mapping.name) - 1] = '\0';
    mapping.callback = callback;
    mymap.used++;
  }
}

int JsonRPC::processMessage(uint8_t rpc_type)
{
  bool is_serial_error = ((rpc_type == RPC_TYPE_SERIAL) && (!doc.containsKey(F("id")) || !doc.containsKey(F("method"))));
  bool is_radio_error = ((rpc_type == RPC_TYPE_RADIO) && (!doc.containsKey(F("i")) || !doc.containsKey(F("m"))));
  bool is_https_error = ((rpc_type == RPC_TYPE_HTTPS) && (!doc.containsKey(F("method"))));
  bool is_can_error = ((rpc_type == RPC_TYPE_CAN) && (!doc.containsKey(F("method"))));

  if (is_serial_error || is_radio_error || is_https_error || is_can_error)
  {
    // id or method are missed
    doc.to<JsonObject>(); // clean the doc
    return E_PARSE_ERROR;
  }

  const char *method = doc[(rpc_type == RPC_TYPE_RADIO) ? F("m") : F("method")].as<const char *>();

  if (method == NULL)
  {
    doc.to<JsonObject>(); // clean the doc
    return E_PARSE_ERROR;
  }

  int ret_status = E_METHOD_NOT_FOUND;

  for (unsigned int i = 0; i < mymap.used; i++)
  {
    mapping = &mymap.mappings[i];
    if (strcmp(method, mapping->name) == 0)
    {
      doc.remove((rpc_type == RPC_TYPE_RADIO) ? F("m") : F("method"));
      return E_BUSY;
    }
  }

  if (ret_status != E_BUSY && ret_status != E_SUCCESS)
  {
    // jsonrpc response with error and message

    doc.remove((rpc_type == RPC_TYPE_RADIO) ? F("m") : F("method"));
    doc.remove((rpc_type == RPC_TYPE_RADIO) ? F("p") : F("params"));
    doc.remove((rpc_type == RPC_TYPE_RADIO) ? F("r") : F("result"));
    JsonObject error = doc.createNestedObject((rpc_type == RPC_TYPE_RADIO) ? "e" : "error");
    error[(rpc_type == RPC_TYPE_RADIO) ? F("c") : F("code")] = ret_status;

    if (rpc_type != RPC_TYPE_RADIO)
    {
      error[F("message")] = jsstrerror(ret_status);
    }
  }

  return ret_status;
}

void JsonRPC::parseCharpointer(bool *is_active, char *rpcin, const size_t rpcin_len, char *rpcout, const size_t rpcout_len, uint8_t rpc_type)
{
  int status;
  DeserializationError error;

  switch (jrpc_state)
  {
  case JRPC_INIT:
    *is_active = true;
    jrpc_state = JRPC_AVAILABLE;
    break;

  case JRPC_AVAILABLE:
    error = deserializeJson(doc, rpcin, rpcin_len);
    if (error)
    {
      jrpc_state = JRPC_END;
    }
    else
    {
      status = processMessage(rpc_type);
      if (status != E_BUSY && status != E_SUCCESS)
      {
        if (rpcout != NULL)
        {
          serializeJson(doc, rpcout, rpcout_len);
        }
        jrpc_state = JRPC_END;
      }
      else
      {
        jrpc_state = JRPC_PROCESS;
      }
    }
    break;

  case JRPC_PROCESS:
    status = callback(rpc_type);

    if (status != E_BUSY)
    {
      if (rpcout != NULL)
      {
        serializeJson(doc, rpcout, rpcout_len);
      }
      jrpc_state = JRPC_END;
    }
    break;

  case JRPC_END:
    jrpc_state = JRPC_INIT;
    *is_active = false;
    break;
  }
}

void JsonRPC::parseStream(bool *is_active, Stream *stream, const uint32_t timeout, uint8_t rpc_type)
{
  int status;
  DeserializationError error;

  switch (jrpc_state)
  {
  case JRPC_INIT:
    stream->setTimeout(timeout);
    if (stream->available())
    {
      *is_active = true;
      jrpc_state = JRPC_AVAILABLE;
    }
    else
    {
      jrpc_state = JRPC_END;
    }
    break;

  case JRPC_AVAILABLE:
    stream->setTimeout(timeout);
    error = deserializeJson(doc, *stream);

    if (error)
    {
      while (stream->available())
      {
        stream->read(); // clean stream
      }

      jrpc_state = JRPC_END;
    }
    else
    {
      status = processMessage(rpc_type);
      if (status != E_BUSY && status != E_SUCCESS)
      {
        serializeJson(doc, *stream);
        jrpc_state = JRPC_END;
      }
      else
      {
        jrpc_state = JRPC_PROCESS;
      }
    }
  break;

  case JRPC_PROCESS:
    status = callback(rpc_type);

    if (status != E_BUSY)
    {
      jrpc_state = JRPC_END;
      serializeJson(doc, *stream);
    }
    break;

  case JRPC_END:
    jrpc_state = JRPC_INIT;
    *is_active = false;
    break;
  }
}

int JsonRPC::callback(uint8_t rpc_type)
{
  JsonObject params = doc[(rpc_type == RPC_TYPE_RADIO) ? F("p") : F("params")];
  JsonObject result = doc.createNestedObject((rpc_type == RPC_TYPE_RADIO) ? F("r") : F("result"));

  int ret_status = mapping->callback(params, result);

  doc.remove((rpc_type == RPC_TYPE_RADIO) ? F("p") : F("params"));

  if ((ret_status != E_BUSY) && (ret_status != E_SUCCESS))
  {
    doc.remove((rpc_type == RPC_TYPE_RADIO) ? F("r") : F("result"));
    JsonObject error = doc.createNestedObject((rpc_type == RPC_TYPE_RADIO) ? "e" : "error");
    error[(rpc_type == RPC_TYPE_RADIO) ? F("c") : F("code")] = ret_status;

    if (rpc_type != RPC_TYPE_RADIO)
    {
      error[F("message")] = jsstrerror(ret_status);
    }
  }

  return ret_status;
}

const __FlashStringHelper *jsstrerror(int errnum)
{
  /* use this to provide a perror style method to help consumers out */
  struct _errordesc
  {
    int code;
    const __FlashStringHelper *message;
  } errordesc[] = {
      {E_SUCCESS, F("No error")},
      {E_PARSE_ERROR, F("Parse error")},           // Invalid JSON was received by the server. An error occurred on the server while parsing the JSON text.
      {E_INVALID_REQUEST, F("Invalid Request")},   // The JSON sent is not a valid Request object.
      {E_METHOD_NOT_FOUND, F("Method not found")}, // The method does not exist / is not available.
      {E_INVALID_PARAMS, F("Invalid param")},      // Invalid method parameter(s).
      {E_INTERNAL_ERROR, F("Internal error")},     // Internal JSON-RPC error.
      {E_BUSY, F("Busy")}
      // -32000 to -32099 	Server error 	Reserved for implementation-defined server-errors.
  };

#define N_ELEMENTS(array) (sizeof(array) / sizeof((array)[0]))

  for (uint8_t i = 0; i <= N_ELEMENTS(errordesc); i++)
  {
    if (errordesc[i].code == errnum)
    {
      return errordesc[i].message;
    }
  }

  return F("");
}
