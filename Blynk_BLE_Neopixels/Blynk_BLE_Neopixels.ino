
//Leave these at the top, becase C++.
//#define BLYNK_DEBUG
//#define BLYNK_PRINT Serial
#define BLYNK_USE_DIRECT_CONNECT

#include <SoftwareSerial.h>
#include <BlynkSimpleSerialBLE.h>

#include <SPI.h>
#include <FastLED.h>

#include <Adafruit_BLE.h>
#include <Adafruit_BluefruitLE_SPI.h>

#include "GaffUtils.h"
#include "CFairy.h"

//===============
// Blynk setup
//===============

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "7ab3a74d83574db18edf902f5d619653";

//===============
// BLE setup
//===============

// SHARED SPI SETTINGS (see adafruit webpages for details)
#define BLUEFRUIT_SPI_CS               8
#define BLUEFRUIT_SPI_IRQ              7
#define BLUEFRUIT_SPI_RST              4    // Optional but recommended, set to -1 if unused
#define BLUEFRUIT_VERBOSE_MODE         true

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);


//===============
// Fast LED setup
//===============

#define NUM_LEDS 89
#define LED_PIN 18
#define COLOR_ORDER GRB
#define CHIPSET     WS2811
CRGB g_leds[NUM_LEDS];
CHSV g_zergba = {33, 255, 255};
uint8_t g_mode = 2;
uint8_t g_tickspeed = 40;
uint8_t g_length = 5;
bool g_dir = 1;
uint8_t g_wash = 0;

void setup() {
  delay(1500);
  Serial.begin(115200);
  Serial.println(F("Blynk for adafruit BLE modules with FAST LED neopixels")); 
  
  Blynk.begin(auth, ble);
  ble.begin(BLUEFRUIT_VERBOSE_MODE);   
  //ble.factoryReset(); //Optional
  ble.setMode(BLUEFRUIT_MODE_DATA); 
  
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(g_leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(50);
  fill_solid( g_leds, NUM_LEDS, CRGB::Black );
  //coming in the next version!
  //FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);  
  FastLED.show();   
}

/*
//Writes are buggered at the moment. See: http://community.blynk.cc/t/ble-pushes-not-working-reliably/6416
 
BLYNK_CONNECTED() {  
  Serial.println(F("Reconnecting"));
  Blynk.virtualWrite(V1, mode);
  Blynk.virtualWrite(V2, tickspeed);  
  Blynk.virtualWrite(V21, mode);
  Blynk.virtualWrite(V22, tickspeed);    
  //CRGB c = CRGB(zergba);
  //Blynk.virtualWrite(V3, c.r, c.g, c.b );
}
*/

BLYNK_WRITE(V3) //Zergba
{  
  CRGB c = CRGB(param[0].asInt(), param[1].asInt(), param[2].asInt());
  g_zergba = rgb2hsv_approximate(c);
  g_zergba = CHSV(g_zergba.h, g_zergba.s, 255);
}

BLYNK_WRITE(V10) //Random
{
  int pinData = param.asInt();
  if( pinData == 0)
    g_zergba = CHSV(random8(), 255, 255);
}

BLYNK_WRITE(V1) //dropdown.
{
  g_mode = param.asInt(); 
}

BLYNK_WRITE(V2) //speed
{
  g_tickspeed = param.asInt();
}

BLYNK_WRITE(V4) //length
{
  g_length = param.asInt();
}

BLYNK_WRITE(V5) //brightness
{
  int b = param.asInt();
  if(b<0) b = 0;
  if(b>120) b = 120;
  FastLED.setBrightness(b);
}

BLYNK_WRITE(V6) //wash
{
  int b = param.asInt();
  if(b<0) b = 0;
  if(b>50) b = 50;
  g_wash = b;
}

void render() {
  switch(g_mode) {
    case 0: //Blynk is 1-indexed
    case 1:
      render_fill();
      break;
    case 2:
      render_wave();
      break;
    //chase 1-3
    case 3:
    case 4:
    case 5:
      render_chase();
      break;
    case 6:
      render_fairy();
      break;
  }
}

void wash() {
  fill_solid( g_leds, NUM_LEDS, CHSV(g_zergba.h, g_zergba.s, g_wash));
}

void render_fill() {
  fill_solid( g_leds, NUM_LEDS, g_zergba );
}

void render_wave() {
  int b = beatsin8(g_tickspeed);
  if( b < g_wash) b = g_wash;
  //Serial.print(F("Beat: ")); Serial.println(b);
  fill_solid( g_leds, NUM_LEDS, CHSV( g_zergba.h, g_zergba.s, b ) );
}

void render_chase() {    
  int b;
  if (g_dir)
    b = map( beat16(g_tickspeed), 0, 65535, 0, NUM_LEDS*16);
  else
    b = map( beat16(g_tickspeed), 0, 65535, NUM_LEDS*16, 0);
  
  wash();
  drawFractionalBar(g_leds, NUM_LEDS, b, g_length, g_zergba );

  if(g_mode == 4) {
    drawFractionalBar(g_leds, NUM_LEDS, b + NUM_LEDS*16/2, g_length, g_zergba );
  }
  else if(g_mode == 5) {
    drawFractionalBar(g_leds, NUM_LEDS, b + NUM_LEDS*16/3, g_length, g_zergba );
    drawFractionalBar(g_leds, NUM_LEDS, b + NUM_LEDS*16*2/3, g_length, g_zergba );
  }
}

void render_fairy() {  
  static CFairy<NUM_LEDS/2> faries(NUM_LEDS);
  static CFairy<NUM_LEDS/16> sparkles(NUM_LEDS);

  wash();
  uint16_t b = beat16(g_tickspeed/5);
  faries.render(g_leds, NUM_LEDS, b, g_zergba);
  sparkles.render(g_leds, NUM_LEDS, beat16(g_tickspeed), CHSV(0, 0, 255));  
}

void loop() {
  Blynk.run();
  render();
  FastLED.show();
}
