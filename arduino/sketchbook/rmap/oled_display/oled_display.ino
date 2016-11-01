#include <aJSON.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// SCL GPIO5
// SDA GPIO4
#define OLED_RESET 0  // GPIO0
Adafruit_SSD1306 display(OLED_RESET);

#define XPOS 0
#define YPOS 1
#define DELTAY 2

//File with settings for wifi and rmap server address and topic
#include "settings.h"


WiFiClient espClient;
PubSubClient client(espClient);

aJsonObject *jsondisplay;

#if (SSD1306_LCDHEIGHT != 48)
#error("Height incorrect, use https://github.com/mcauser/Adafruit_SSD1306/tree/esp8266-64x48 lib");
#endif

void setup()   {
  Serial.begin(115200);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  jsondisplay=aJson.createObject();
  
  setup_wifi();
  aJson.addStringToObject(jsondisplay, "B10004_u"," hPa");
  aJson.addStringToObject(jsondisplay, "B12101_u"," C");
  aJson.addStringToObject(jsondisplay, "B13003_u"," %");
  aJson.addStringToObject(jsondisplay, "B15195_u","ug/m3");
  aJson.addStringToObject(jsondisplay, "B15198_u","ug/m3");
  aJson.addNumberToObject(jsondisplay, "B10004_v",0);
  aJson.addNumberToObject(jsondisplay, "B12101_v",0);
  aJson.addNumberToObject(jsondisplay, "B13003_v",0);
  aJson.addNumberToObject(jsondisplay, "B15195_v",0);
  aJson.addNumberToObject(jsondisplay, "B15198_v",0);
}


void callback(char* topic, byte* payload, unsigned int length) {
  float f_val;
  int i_val;
  char* unit;
  aJsonObject* linea;

  Serial.print("Topic= ");
  Serial.print(topic);
  String s_topic = String((char*)topic);
  String s = String((char*)payload);
  Serial.print(" Payload= ");
  Serial.println(s);
  aJsonObject *root = aJson.parse((char*)payload);
  aJsonObject* valore = aJson.getObjectItem(root, "v");
  int BCODE_pos = s_topic.lastIndexOf('/');
  display.setTextSize(1);

  if (s_topic.substring(BCODE_pos + 1) == "B10004") {
    aJson.deleteItemFromObject(jsondisplay, "B10004_v");
    aJson.addNumberToObject(jsondisplay, "B10004_v",(int)(valore -> valueint/100));
    aJson.addStringToObject(jsondisplay, "B10004_u"," 2322");
  }  else if (s_topic.substring(BCODE_pos + 1) == "B12101") {
    aJson.deleteItemFromObject(jsondisplay, "B12101_v");
    aJson.addNumberToObject(jsondisplay, "B12101_v",(float)((valore ->valueint-27315)/100));
  } else if (s_topic.substring(BCODE_pos + 1) == "B13003") {
    aJson.deleteItemFromObject(jsondisplay, "B13003_v");
    aJson.addNumberToObject(jsondisplay, "B13003_v",(int)valore -> valueint);
  } else if (s_topic.substring(BCODE_pos + 1) == "B15195") {
    aJson.deleteItemFromObject(jsondisplay, "B15195_v");
    aJson.addNumberToObject(jsondisplay, "B15195_v",(float)(valore -> valueint/10));
  } else if (s_topic.substring(BCODE_pos + 1) == "B15198") {
    aJson.deleteItemFromObject(jsondisplay, "B15198_v");
    aJson.addNumberToObject(jsondisplay, "B15198_v",(float)(valore -> valueint/10));
  } else {
    return;
  }
  prepare_line(0);
  
  
  linea = aJson.getObjectItem(jsondisplay, "B10004_v");
  Serial.print(linea->valueint);
  display.print(linea->valueint);
  linea = aJson.getObjectItem(jsondisplay, "B10004_u");
  Serial.println(linea->valuestring);
  display.println(linea->valuestring);
  prepare_line(1);
  linea = aJson.getObjectItem(jsondisplay, "B12101_v");
  Serial.print(linea->valuefloat);
  display.print(linea->valuefloat);
  linea = aJson.getObjectItem(jsondisplay, "B12101_u");
  Serial.println(linea->valuestring);
  display.println(linea->valuestring);
  prepare_line(2);
  linea = aJson.getObjectItem(jsondisplay, "B13003_v");
  Serial.print(linea->valueint);
  display.print(linea->valueint);
  linea = aJson.getObjectItem(jsondisplay, "B13003_u");
  Serial.println(linea->valuestring);
  display.println(linea->valuestring);
  prepare_line(3);
  linea = aJson.getObjectItem(jsondisplay, "B15195_v");
  Serial.print(linea->valuefloat);
  display.print(linea->valuefloat);
  linea = aJson.getObjectItem(jsondisplay, "B15195_u");
  Serial.println(linea->valuestring);
  display.println(linea->valuestring);
  prepare_line(4);
  linea = aJson.getObjectItem(jsondisplay, "B15198_v");
  Serial.print(linea->valuefloat);
  display.print(linea->valuefloat);
  linea = aJson.getObjectItem(jsondisplay, "B15198_u");
  Serial.println(linea->valuestring);
  display.println(linea->valuestring);
  display.display();
//  char* jsondbg = aJson.print(jsondisplay);
//  Serial.println(jsondbg);
//  free(jsondbg);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Conn mqtt");
    display.println(mqtt_server);
    display.display();
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.println("connected");
      display.display();
      // ... and resubscribe
      client.subscribe(mqtt_topic);
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.println("Subscribed");
      display.display();
      delay(1000);
      display.clearDisplay();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("failed, rc=");
      display.println(client.state());
      display.println("retry 5 sec");
      display.display();
      delay(5000);
    }
  }
}

void prepare_line(int line_num) {
  display.fillRect(0, line_num * 9, display.width() - 1, ((line_num + 1) * 9) - 1, BLACK);
  display.setCursor(0, line_num * 9);
}


void setup_wifi() {
  int x = 0;
  display.println("Conn. to:");
  display.println(ssid);
  display.display();

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    display.setTextSize(1);
    display.print(".");
    display.display();
    x++;
  }
  display.clearDisplay();
  display.setCursor(0, 0);
  delay(300);
  display.println("WiFi ok");
  display.print("IP: ");
  display.println(WiFi.localIP());
  display.display();
  delay(2000);
  display.clearDisplay();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    setup();
  }
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

}
