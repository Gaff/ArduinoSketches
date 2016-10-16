//#define BLYNK_DEBUG
#define BLYNK_PRINT Serial

#include <blynk.h>
#include <application.h>;

#include <SoftwareSerial.h>
#include <BlynkSimpleSerialBLE.h>

#include <SPI.h>
#include <FastLED.h>


char auth[] = "...";

#define NUM_LEDS 20
#define LED_PIN 18
#define COLOR_ORDER GRB
#define CHIPSET     WS2811
CRGB g_leds[NUM_LEDS];
CHSV g_zergba = {33, 255, 255};

void setup() {  
  Serial.begin(115200);
  
  Blynk.begin(auth);

  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(g_leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(100); //optional
  fill_solid( g_leds, NUM_LEDS, CRGB::Black );
  
  FastLED.show();   
}


BLYNK_WRITE(V3) //Zergba
{  
  CRGB c = CRGB(param[0].asInt(), param[1].asInt(), param[2].asInt());
  g_zergba = rgb2hsv_approximate(c);
  g_zergba = CHSV(g_zergba.h, g_zergba.s, 255);
  
}



void loop() {
  Blynk.run();
  fill_solid( g_leds, NUM_LEDS, g_zergba );
  FastLED.show();
}
