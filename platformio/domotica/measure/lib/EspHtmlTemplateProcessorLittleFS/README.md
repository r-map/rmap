# Web page template processing for ESP using SPIFFS

The purpose of this library is to handle template processing of web page for an ESP using SPIFFS.
It works with "ESP8266WebServer" (or "WebServer" for ESP32) included in the ESP arduino SDK.

The base idea come from the [ESPTemplateProcessor](https://github.com/winder/ESPTemplateProcessor) library.
However, it's a complete rewrite that allow memory optimisation and new functionnalities such as escaping and syntax error detection.

Tested with an ESP8266 (Wemos D1 mini), should work with ESP32 but yet untested.

## How does it work

Templates are regular html pages that contain keywords surrounded by double curly brackets.
These keywords will by passed to a function inside you sketch and will be substitued by the return value.

Let's take a simple html page, index.html

```html
<html>
  <title>An exemple web page</title>
  <body>
    <h1>{{TITLE}}</h1>
    <p>{{VAR1}}</p>
  </body>
</html>
```

This page has two templating keywords : TITLE and VAR1.
Lets look to how we manage that in an arduino sketch.

```c++
// Returning the substitution value of a specific keyword
const char* indexKeyProcessor(const String& key)
{
  if (key == "TITLE") return "Hello World!";
  else if (key == "VAR1") return "It works!";

  return "Key not found";
}

// Send the web page over http, referencing the indexKeyProcessor function
templateProcessor.processAndSend("/index.html", indexKeyProcessor);
```

Take note that it is possible to escape double curly brackets by adding a \ after. For exemple, {{\test}} will not be treated as a templating keyword and will be displayed in your web page as {{test}}

To sse the complete exemple that illustrate how all is tied together, see the provided exemple sketch.

## Running the provided example

- First, if not already done, follow these [steps](https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html#uploading-files-to-file-system) to upload your web pages to the SPIFFS of your ESP using Arduino IDE. This will write files and folder contained into the /data directory of your sketch folder to the SPIFFS of your ESP.
- Second, you need to upload the provided sketch "SimpleHtmlTemplate.ino" to your board.
- Now, browse to the ip of your esp (the ip is displayed trough the serial console), done !

## Going further

I developped this to have a simple and lite templating engine that works with the included web server of the ESP SDK.
However, if you have a project more complex than just few simple web pages, I suggest you to take a look to the [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) library.
The ESPAsyncWebServer replace the provided web server and include his own templating engine and provide functions to automatically serve static files/folder from SPIFFS with many more improvment and functionalities.


