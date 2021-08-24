#ifndef ESP_HTML_TEMPLATE_PROCESSOR_H
#define ESP_HTML_TEMPLATE_PROCESSOR_H

#ifdef ESP8266
#define WebServer ESP8266WebServer
#include <ESP8266WebServer.h>
#else
#include <WebServer.h>
#endif

#include "FileReader.h"

#define OPENING_CURLY_BRACKET_CHAR '{'
#define CLOSING_CURLY_BRACKET_CHAR '}'
#define ESCAPE_CHAR 92 // char: '\'
#define BUFFER_SIZE 100 // Buffer size for file reading

typedef String (*const GetKeyValueCallback) (const String& key);

class EspHtmlTemplateProcessor
{
private:
  WebServer* mServer;

public:
  EspHtmlTemplateProcessor(WebServer* server);
  ~EspHtmlTemplateProcessor();

  bool processAndSend(const String& filePath, GetKeyValueCallback getKeyValueCallback);

private:
  void sendError(const String& errorDescription) const;
};

#endif