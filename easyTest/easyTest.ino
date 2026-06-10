#include <Arduino.h>
#include <FastLED.h>

#define NUM_LEDS 5                                          //количество светодиодов
#define DATA_PIN D3                                         //пин
CRGB leds[NUM_LEDS];                                        //создаем массив размером в NUM_LEDS элементов

void setup() {
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);     //инициализируем объект leds
}

void loop() {
  leds[0] = CRGB::Red;
  leds[1] = CRGB(0, 0, 255);
  leds[2] = CRGB(255, 255, 255);
  FastLED.show();
  delay(500);
  leds[0] = CRGB(255,255,255);
  leds[1] = CRGB(0,0,255);
  leds[2] = CRGB(255,0,0);
  leds[3] = CRGB::SeaGreen;
  leds[4] = CRGB::SeaGreen;
  FastLED.show();
  delay(500);
}