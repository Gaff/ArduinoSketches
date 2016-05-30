
//Leave these at the top, becase C++.
#define BLYNK_DEBUG
#define BLYNK_PRINT Serial
#define BLYNK_USE_DIRECT_CONNECT

#include <SoftwareSerial.h>
#include <BlynkSimpleSerialBLE.h>

#include <SPI.h>
#include <FastLED.h>

#include <Adafruit_BLE.h>
#include <Adafruit_BluefruitLE_SPI.h>

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

#define NUM_LEDS 32
#define LED_PIN 18
#define COLOR_ORDER GRB
#define CHIPSET     WS2811
CRGB leds[NUM_LEDS];
CHSV zergba = {33, 255, 255};
uint8_t mode = 0;
uint8_t tickspeed = 80;

void setup() {
  delay(1500);
  Serial.begin(115200);
  Serial.println(F("Blynk for adafruit BLE modules with FAST LED neopixels")); 
  
  Blynk.begin(auth, ble);
  ble.begin(BLUEFRUIT_VERBOSE_MODE);   
  //ble.factoryReset(); //Optional
  ble.setMode(BLUEFRUIT_MODE_DATA); 
  
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(100);
  fill_solid( leds, NUM_LEDS, CRGB::Black );
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
  zergba = rgb2hsv_approximate(c);
  zergba = CHSV(zergba.h, zergba.s, 255);
}

BLYNK_WRITE(V10) //Random
{
  int pinData = param.asInt();
  if( pinData == 0)
    zergba = CHSV(random8(), 255, 255);
}

BLYNK_WRITE(V1) //dropdown.
{
  mode = param.asInt(); 
}

BLYNK_WRITE(V2) //speed
{
  tickspeed = param.asInt();
}

void render() {
  switch(mode) {
    case 0: //Blynk is 1-indexed
    case 1:
      render_fill();
      break;
    case 2:
      render_wave();
      break;
  }
}

void render_fill() {
  fill_solid( leds, NUM_LEDS, zergba );
}

void render_wave() {
  int b = beatsin8(tickspeed);
  //Serial.print(F("Beat: ")); Serial.println(b);
  fill_solid( leds, NUM_LEDS, CHSV( zergba.h, zergba.s, b ) );
}

void loop() {
  Blynk.run();
  render();
  FastLED.show();
}
