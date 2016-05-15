#include <Arduino.h>
#include <FastLED.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define NUM_LEDS 20
#define LED_PIN 0
#define WASH 0
#define GEMSIZE 7

#define COLOR_ORDER GRB
#define CHIPSET     WS2811

CRGB leds[NUM_LEDS];



void setup() {
  // put your setup code here, to run once:
  delay(1500);// sanity delay 
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif  
  memset(leds, 0, sizeof( leds) );
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(255);
  fill_solid( leds, NUM_LEDS, CRGB::Black );
  FastLED.show();   
}


static void shuffle(uint8_t *data) {
  for(uint8_t i = 0; i < GEMSIZE-1; i++) {
    uint8_t c = random8(GEMSIZE-i);
    uint8_t t = data[i]; data[i] = data[i+c]; data[i+c] = t;  /* swap */
  }
}

static void sparkle() {

  static CHSV c = CHSV(108, 255, WASH);
  static uint8_t phase = 0;  
  static int8_t inc = 1;
  static uint8_t sparkles[GEMSIZE] = {0,1,2,3,4,5,6};

  uint8_t t = beat8( 80 );

  fill_solid( leds, NUM_LEDS, CHSV( c.h, c.s, t ) );
  if(phase == 0) shuffle(sparkles);
  if(t<10) c = CHSV(random8(), 255, WASH);
  phase++; 
  
}

void loop() {
  // put your main code here, to run repeatedly:
  sparkle();
  FastLED.show();
}
