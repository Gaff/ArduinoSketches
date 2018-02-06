#include "FastLED.h"
#define NUM_LEDS 64 

#define DATA_PIN 6
CRGB leds[NUM_LEDS];


void setup() { 
  Serial.begin(57600);
  Serial.println("resetting");
  LEDS.addLeds<WS2812,DATA_PIN,GRB>(leds,NUM_LEDS);
  LEDS.setBrightness(180);
}

uint8_t bri = 0;
uint8_t hugh = 0;
int phase = 0;
int cycle = 0;
int dir = 1;

void loop() {  


  if(phase == 1) {
    Serial.println("phase 1");
    delay(1200);
    phase++;
    cycle = 0;
    dir = -1;
  }
  else if(phase == 3) {
    Serial.println("phase 3");
    delay(1200);
    phase++;
    cycle = 0;
    dir = 1;
  }
  else if(phase == 0 || phase == 2) {
    Serial.println("phasing...");
    bri = bri + dir;
    cycle = cycle+2;
    if( cycle >= 255 ) {
      cycle = 0;
      phase++;    
    }
  }
  
  if(phase == 4) {
    hugh = hugh + 85;
    phase = 0;
  }
  
  for(int i = 0; i < NUM_LEDS; i++) {
    // Set the i'th led to red 
    leds[i] = CHSV(hugh, 255, bri);
    // Show the leds
  }
  
  FastLED.show(); 
  delay(10);

}
