#include <SoftwareSerial.h>
#include "FastLED.h"


#define PIN 18
#define NUM_LEDS 40
CRGB leds[NUM_LEDS];

void setup() { 
  delay(3000);
  //while (!Serial);  // required for Flora & Micro

  Serial.begin(115200);
  
  Serial.println(F("MSH FastLED Yay"));
  Serial.println(F("-----------------------------------------"));
    
  FastLED.addLeds<NEOPIXEL, PIN>(leds, NUM_LEDS); 
  FastLED.setBrightness( 100 );  
}

void loop() { 
  
  fadeToBlackBy( leds, NUM_LEDS, 25);
  int i = random16(NUM_LEDS);
  leds[i] += CHSV( random8(), 255, 255);
  Serial.println(F("loop"));
  FastLED.show();
  delay(100);
  
}
