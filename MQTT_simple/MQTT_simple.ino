#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <FastLED.h>

const char* ssid = "Test";
const char* password = "12345678";

// =======================
// MQTT WQTT
// =======================
const char* mqtt_server = "m6.wqtt.ru";
const int mqtt_port = 19940;
const char* mqtt_user = "u_KXTXIO";
const char* mqtt_password = "";

// =======================
// MQTT топики WQTT
// =======================
// WQTT выключатель питания:
// Топик управления: lampa
// Топик состояния: state
// Команда включить: 1
// Команда выключить: 0

const char* TOPIC_POWER  = "lampa";
const char* TOPIC_STATE  = "state";
const char* TOPIC_BRIGHT = "lampa/brig";
const char* TOPIC_COLOR  = "lampa/color";

// =======================
// LED лента
// =======================
#define DATA_PIN D3
#define LED_COUNT 5
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

CRGB leds[LED_COUNT];

// =======================
// Состояние ленты
// =======================
bool led_on = false;
int led_brig = 50;              // 0...100
String led_color = "#FFFFFF";   // HEX

WiFiClient espClient;
PubSubClient client(espClient);

// =======================
// Печать состояния
// =======================
void printCurrentState() {
  Serial.println("----- CURRENT STATE -----");
  Serial.print("led_on: ");
  Serial.println(led_on ? "true" : "false");

  Serial.print("led_brig: ");
  Serial.println(led_brig);

  Serial.print("led_color: ");
  Serial.println(led_color);

  Serial.print("WiFi status: ");
  Serial.println(WiFi.status());

  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  Serial.print("MQTT connected: ");
  Serial.println(client.connected() ? "true" : "false");
  Serial.println("-------------------------");
}

// =======================
// Установка всей ленты
// =======================
void setAll(byte r, byte g, byte b) {
  Serial.print("setAll raw RGB: ");
  Serial.print(r);
  Serial.print(", ");
  Serial.print(g);
  Serial.print(", ");
  Serial.println(b);

  r = r * led_brig / 100;
  g = g * led_brig / 100;
  b = b * led_brig / 100;

  Serial.print("setAll brightness RGB: ");
  Serial.print(r);
  Serial.print(", ");
  Serial.print(g);
  Serial.print(", ");
  Serial.println(b);

  for (int i = 0; i < LED_COUNT; i++) {
    leds[i] = CRGB(r, g, b);
  }

  FastLED.show();
  Serial.println("FastLED.show() done");
}

// =======================
// Применить состояние ленты
// =======================
void updateLed() {
  Serial.println("updateLed()");

  if (!led_on) {
    Serial.println("LED state: OFF");
    setAll(0, 0, 0);
    return;
  }

  Serial.println("LED state: ON");

  if (led_color.length() != 7 || led_color[0] != '#') {
    Serial.println("Wrong led_color format, fallback to #FFFFFF");
    led_color = "#FFFFFF";
  }

  long color = strtol(led_color.substring(1).c_str(), NULL, 16);

  byte r = (color >> 16) & 0xFF;
  byte g = (color >> 8) & 0xFF;
  byte b = color & 0xFF;

  Serial.print("Parsed color RGB: ");
  Serial.print(r);
  Serial.print(", ");
  Serial.print(g);
  Serial.print(", ");
  Serial.println(b);

  setAll(r, g, b);
}

// =======================
// Публикация состояния обратно в WQTT
// =======================
void publishState() {
  Serial.println("publishState()");

  if (client.connected()) {
    bool ok = client.publish(TOPIC_STATE, led_on ? "1" : "0", true);

    Serial.print("Publish topic: ");
    Serial.println(TOPIC_STATE);

    Serial.print("Publish payload: ");
    Serial.println(led_on ? "1" : "0");

    Serial.print("Publish result: ");
    Serial.println(ok ? "OK" : "FAILED");
  } else {
    Serial.println("Publish skipped: MQTT not connected");
  }
}

// =======================
// Подключение к Wi-Fi
// =======================
void setup_wifi() {
  delay(100);

  Serial.println();
  Serial.println("===== WIFI SETUP =====");
  Serial.print("SSID: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int attempts = 0;

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    attempts++;

    if (attempts % 10 == 0) {
      Serial.println();
      Serial.print("WiFi status: ");
      Serial.println(WiFi.status());
    }

    if (attempts > 60) {
      Serial.println();
      Serial.println("Wi-Fi connection timeout. Restarting...");
      ESP.restart();
    }
  }

  Serial.println();
  Serial.println("Wi-Fi connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());

  Serial.print("DNS: ");
  Serial.println(WiFi.dnsIP());

  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());

  Serial.println("======================");
}

// =======================
// Обработка входящих MQTT сообщений
// =======================
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println();
  Serial.println("===== MQTT MESSAGE =====");

  String topicStr = String(topic);
  String msg = "";

  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  msg.trim();

  Serial.print("Topic: ");
  Serial.println(topicStr);

  Serial.print("Payload length: ");
  Serial.println(length);

  Serial.print("Payload: ");
  Serial.println(msg);

  // =======================
  // Включение / выключение
  // =======================
  if (topicStr == TOPIC_POWER) {
    Serial.println("Matched: TOPIC_POWER");

    if (msg == "1" || msg == "ON" || msg == "on" || msg == "true") {
      Serial.println("Command: POWER ON");
      led_on = true;
    } else if (msg == "0" || msg == "OFF" || msg == "off" || msg == "false") {
      Serial.println("Command: POWER OFF");
      led_on = false;
    } else {
      Serial.println("Unknown POWER command");
    }

    updateLed();
    publishState();
    printCurrentState();
  }

  // =======================
  // Яркость
  // =======================
  else if (topicStr == TOPIC_BRIGHT) {
    Serial.println("Matched: TOPIC_BRIGHT");

    int new_brig = msg.toInt();

    Serial.print("Received brightness: ");
    Serial.println(new_brig);

    if (new_brig < 0) new_brig = 0;
    if (new_brig > 100) new_brig = 100;

    led_brig = new_brig;

    Serial.print("Applied brightness: ");
    Serial.println(led_brig);

    updateLed();
    printCurrentState();
  }

  // =======================
  // Цвет
  // =======================
  else if (topicStr == TOPIC_COLOR) {
    Serial.println("Matched: TOPIC_COLOR");

    if (msg.length() == 7 && msg[0] == '#') {
      led_color = msg;

      Serial.print("Applied color: ");
      Serial.println(led_color);

      updateLed();
    } else {
      Serial.println("Wrong color format. Expected #RRGGBB");
    }

    printCurrentState();
  }

  else {
    Serial.println("Unknown topic");
  }

  Serial.println("========================");
}

// =======================
// Подключение к MQTT
// =======================
void reconnect() {
  while (!client.connected()) {

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Wi-Fi lost. Reconnecting...");
      setup_wifi();
    }

    Serial.println();
    Serial.println("===== MQTT CONNECT =====");

    Serial.print("Server: ");
    Serial.println(mqtt_server);

    Serial.print("Port: ");
    Serial.println(mqtt_port);

    Serial.print("User: ");
    Serial.println(mqtt_user);

    Serial.print("WiFi status: ");
    Serial.println(WiFi.status());

    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    Serial.print("RSSI: ");
    Serial.println(WiFi.RSSI());

    String clientId = "ESP8266-";
    clientId += String(ESP.getChipId());
    clientId += "-";
    clientId += String(random(0xffff), HEX);

    Serial.print("Client ID: ");
    Serial.println(clientId);

    Serial.print("MQTT connecting... ");

    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected");

      bool s1 = client.subscribe(TOPIC_POWER);
      bool s2 = client.subscribe(TOPIC_BRIGHT);
      bool s3 = client.subscribe(TOPIC_COLOR);

      Serial.println("Subscribed:");
      Serial.print(TOPIC_POWER);
      Serial.print(" -> ");
      Serial.println(s1 ? "OK" : "FAILED");

      Serial.print(TOPIC_BRIGHT);
      Serial.print(" -> ");
      Serial.println(s2 ? "OK" : "FAILED");

      Serial.print(TOPIC_COLOR);
      Serial.print(" -> ");
      Serial.println(s3 ? "OK" : "FAILED");

      publishState();
      printCurrentState();

    } else {
      Serial.println("failed");

      Serial.print("MQTT rc: ");
      Serial.println(client.state());

      Serial.println("rc meanings:");
      Serial.println("-4 timeout");
      Serial.println("-3 connection lost");
      Serial.println("-2 connect failed");
      Serial.println("-1 disconnected");
      Serial.println(" 1 bad protocol");
      Serial.println(" 2 bad client id");
      Serial.println(" 3 server unavailable");
      Serial.println(" 4 bad user/pass");
      Serial.println(" 5 not authorized");

      Serial.println("Retry in 10 seconds...");
      Serial.println("========================");

      delay(10000);
    }
  }
}

// =======================
// SETUP
// =======================
void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println();
  Serial.println("================================");
  Serial.println("ESP8266 WQTT LED STRIP START");
  Serial.println("================================");

  Serial.print("Chip ID: ");
  Serial.println(ESP.getChipId());

  Serial.print("Flash chip size: ");
  Serial.println(ESP.getFlashChipSize());

  Serial.print("Free heap: ");
  Serial.println(ESP.getFreeHeap());

  randomSeed(ESP.getChipId() + micros());

  Serial.println("FastLED init...");
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, LED_COUNT);
  FastLED.clear();
  FastLED.show();
  Serial.println("FastLED ready");

  setup_wifi();

  Serial.println("MQTT client setup...");
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  client.setKeepAlive(30);
  client.setSocketTimeout(15);

  Serial.println("Setup done");
}

// =======================
// LOOP
// =======================
void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();
}