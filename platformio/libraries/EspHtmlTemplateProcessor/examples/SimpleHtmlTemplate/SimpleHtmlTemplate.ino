#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "EspHtmlTemplateProcessor.h"

void handleRoot();

const char* ssid = "WifiSSID";
const char* password = "WifiPassword";

ESP8266WebServer server;
EspHtmlTemplateProcessor templateProcessor(&server);

void setup() 
{
  Serial.begin(115200);
  SPIFFS.begin();

  WiFi.begin(ssid, password);

  Serial.println("Connecting to wifi ");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }

  server.on("/", handleRoot);

  server.begin();
  Serial.println("HTTP server started");

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() 
{
  server.handleClient();
}

const char* indexKeyProcessor(const String& key)
{
  if (key == "TITLE") return "Hello World!";
  else if (key == "VAR1") return "It works!";

  return "Key not found";
}

void handleRoot()
{
  templateProcessor.processAndSend("/index.html", indexKeyProcessor);
}