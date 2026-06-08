#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FastLED.h>

const char* ssid = "Test";
const char* password = "12345678";

#define LED_PIN D3
#define LED_COUNT 5
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

#define BRIGHTNESS 80

CRGB leds[LED_COUNT];

ESP8266WebServer server(80);

void setAll(const CRGB& color) {
  for (int i = 0; i < LED_COUNT; i++) {
    leds[i] = color;
  }
  FastLED.show();
}

void handleRoot() {
  String html = "";
  html += "<h1>Wemos WS2812 FastLED control</h1>";
  html += "<p><a href='/on'>ON</a></p>";
  html += "<p><a href='/off'>OFF</a></p>";
  html += "<p><a href='/red'>RED</a></p>";
  html += "<p><a href='/green'>GREEN</a></p>";
  html += "<p><a href='/blue'>BLUE</a></p>";
  html += "<p><a href='/white'>WHITE</a></p>";
  html += "<p><a href='/yellow'>YELLOW</a></p>";
  html += "<p><a href='/purple'>PURPLE</a></p>";

  server.send(200, "text/html", html);
}

void handleOn() {
  setAll(CRGB::White);
  server.send(200, "text/plain", "ON");
}

void handleOff() {
  setAll(CRGB::Black);
  server.send(200, "text/plain", "OFF");
}

void handleRed() {
  setAll(CRGB::Red);
  server.send(200, "text/plain", "RED");
}

void handleGreen() {
  setAll(CRGB::Green);
  server.send(200, "text/plain", "GREEN");
}

void handleBlue() {
  setAll(CRGB::Blue);
  server.send(200, "text/plain", "BLUE");
}

void handleWhite() {
  setAll(CRGB::White);
  server.send(200, "text/plain", "WHITE");
}

void handleYellow() {
  setAll(CRGB::Yellow);
  server.send(200, "text/plain", "YELLOW");
}

void handlePurple() {
  setAll(CRGB::Purple);
  server.send(200, "text/plain", "PURPLE");
}

void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

void setup() {
  Serial.begin(115200);

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, LED_COUNT);
  FastLED.setBrightness(BRIGHTNESS);
  setAll(CRGB::Black);

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
  server.on("/yellow", handleYellow);
  server.on("/purple", handlePurple);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  FastLED.show();
}