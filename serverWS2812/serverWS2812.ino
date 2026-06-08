#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FastLED.h

const char* ssid = "Test";
const char* password = "12345678";

#define NUM_LEDS 5   //количество светодиодов
#define DATA_PIN D3  //пин
CRGB leds[NUM_LEDS];

ESP8266WebServer server(80);

uint32_t currentColor = 0;

void setAll(uint32_t color) {
  currentColor = color;
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
}

void handleRoot() {
  String html = "";
  html += "<h1>Wemos WS2812 control</h1>";
  html += "<p><a href='/on'>ON</a></p>";
  html += "<p><a href='/off'>OFF</a></p>";
  html += "<p><a href='/red'>RED</a></p>";
  html += "<p><a href='/green'>GREEN</a></p>";
  html += "<p><a href='/blue'>BLUE</a></p>";
  html += "<p><a href='/white'>WHITE</a></p>";
  server.send(200, "text/html", html);
}

void handleOn() {
  setAll(strip.Color(255, 255, 255));
  server.send(200, "text/plain", "ON");
}

void handleOff() {
  setAll(strip.Color(0, 0, 0));
  server.send(200, "text/plain", "OFF");
}

void handleRed() {
  setAll(strip.Color(255, 0, 0));
  server.send(200, "text/plain", "RED");
}

void handleGreen() {
  setAll(strip.Color(0, 255, 0));
  server.send(200, "text/plain", "GREEN");
}

void handleBlue() {
  setAll(strip.Color(0, 0, 255));
  server.send(200, "text/plain", "BLUE");
}

void handleWhite() {
  setAll(strip.Color(255, 255, 255));
  server.send(200, "text/plain", "WHITE");
}

void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  //инициализируем объект leds


  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.println();
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.on("/red", handleRed);
  server.on("/green", handleGreen);
  server.on("/blue", handleBlue);
  server.on("/white", handleWhite);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}