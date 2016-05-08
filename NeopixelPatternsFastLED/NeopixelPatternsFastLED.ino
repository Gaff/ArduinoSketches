#include <string.h>
#include <Arduino.h>
#include <FastLED.h>
#include <SoftwareSerial.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#include <SPI.h>

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

#include "MSHUtils.h"

#define FACTORYRESET_ENABLE         1
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"


struct colour {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};
typedef struct colour Colour;

struct colourPoint {
  uint8_t i;
  Colour colour;
};
typedef struct colourPoint ColourPoint;

// function prototypes over in packetparser.cpp
uint8_t readPacket(Adafruit_BLE *ble, uint16_t timeout);
float parsefloat(uint8_t *buffer);
void printHex(const uint8_t * data, const uint32_t numBytes);

// the packet buffer
extern uint8_t packetbuffer[];

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

#define TOTAL_LEDS 144
#define NUM_LEDS 90
#define LED_PIN 11
#define COLOR_ORDER GRB
#define CHIPSET     WS2811
CRGB all_leds[TOTAL_LEDS];
CRGB *leds; //TODO: Don't need both this and all_leds if we wanted to save a couple of bytes memory.

static uint16_t Pos(uint16_t raw) {
  return (NUM_LEDS + raw) % NUM_LEDS;
}

static int Pos16(int raw) {  
  return (NUM_LEDS*16 + raw) % NUM_LEDS*16;
}

#define WASH 50 //should be something light 40/brightness

void error(const __FlashStringHelper *err) {
  Serial.println(err);
  while (1);
}

template <uint8_t Tpoints, uint8_t Tframes_to_next = 4, uint8_t Tframes_to_max = 16>
class CFaries {  
  uint8_t m_points[Tpoints];   
  uint16_t m_sequence = 0; 
  public:
    uint8_t getPoints() { return Tpoints; }
    
    CFaries() {
      //sanity check:
      //if( FRAMES_TO_MAX > FRAMES_TO_NEXT*getPoints()/2 )
      //  error(F("You misconfigured the faires!"));
      
      //setup the array:
      for(uint8_t i = 0; i < Tpoints; i++ )
        m_points[i] = random(0, NUM_LEDS);
    }
    
    void consider(CHSV c) {
      CHSV dim;      
      
      for(uint8_t i=0;i<getPoints();i++) {        
        uint16_t seqi = m_sequence; //how long i has been alive.
        seqi = (m_sequence + i*Tframes_to_next) % (Tframes_to_next*Tpoints);
        if( seqi == 0 )
          m_points[i] = random(0, NUM_LEDS);
                 
        if( seqi < Tframes_to_max ) {
          dim = CHSV(c.h, c.s, map(seqi, 0, Tframes_to_max, 16, 255 ) ); 
          //strip.setPixelColor( points[i], dim.red, dim.green, dim.blue );
          leds[m_points[i]] |= dim;
        } else if( seqi > Tframes_to_next * Tpoints - Tframes_to_max ) {
          dim = CHSV(c.h, c.s,  map(seqi, Tframes_to_next * Tpoints - Tframes_to_max, Tframes_to_next * Tpoints, 255, 16 ) );       
          //strip.setPixelColor( points[i], dim.red, dim.green, dim.blue ); 
          leds[m_points[i]] |= dim;
        } else {
          //strip.setPixelColor( points[i], c.red, c.green, c.blue );
          leds[m_points[i]] |= CHSV( c.h, c.s, 255);
        }    
      }
    
      m_sequence++;
      if( m_sequence == Tframes_to_next*Tpoints )
        m_sequence = 0;      
    }
};

//Note that the strip is 2xWIDTH wide, half faded in, half faded out.
#define WIDTH 12

class CChase {    
  int m_phase = 0;
  int8_t m_inc = 1;  
  public:
    CChase(uint8_t offset = 0, int8_t inc = 1) {
      m_phase = offset*16;
      m_inc = inc;
    }
  
    void consider(CHSV c) {
        uint16_t j;
 
        drawFractionalBar( leds, NUM_LEDS, m_phase, WIDTH, c);
        m_phase++;
        if( m_phase >= NUM_LEDS * 16 )
          m_phase -= NUM_LEDS*16;
        //m_phase=Pos16(m_phase+m_inc);
    }
    
};

CFaries<28> faries;
CFaries<4, 4, 8> sparkles;
CChase chase;
CChase chase2(NUM_LEDS/2);
CChase chase3(0, -1);

void setup() {
  delay(1500);// sanity delay  
  
  Serial.begin(115200);
  
  Serial.println(F("MSH Neopixels FASTLED Yay"));
  Serial.println(F("-----------------------------------------"));
  Serial.println((long)&all_leds[0]);
  Serial.println((long)&all_leds[1]);
  
  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  
  ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
  ble.setMode(BLUEFRUIT_MODE_DATA);

  Serial.println(F("Bluefruit setup"));
  Serial.println(F("-----------------------------------------"));

  memset(leds, 0, sizeof( leds) );
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(all_leds, TOTAL_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(100);
  fill_solid( all_leds, TOTAL_LEDS, CRGB::Black );
  //coming in the next version!
  //FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);  
  FastLED.show();    

  //For various odd reasons I wish to ignore the first few LEDs....
  leds = &all_leds[TOTAL_LEDS - NUM_LEDS];
  leds[0] = CRGB::Red;
  leds[NUM_LEDS - 1] = CRGB::Blue;
  FastLED.show(); 
  delay(1000);

  //randomFairyInit();  

  Serial.print("Free Ram: " );
  Serial.println(freeRam());
  Serial.println(F("-----------------------------------------"));
}


CHSV colour = {108, 255, 255 };
// the loop function runs over and over again forever
void loop() {  

  static uint8_t index = 0;
  static int8_t inc = 1;
  //Serial.print("Loop: ");
  //Serial.println(micros());

  if (canRead())
  {
    //TODO: Change app to use CHSV
    CRGB c;
    c = optionallyReadColour(c);
    colour =  rgb2hsv_approximate(c);
  }    

  fill_solid(leds, NUM_LEDS, CHSV(colour.h, 255, WASH)); //minimum colour  
  
  //faries.consider(colour);
  //sparkles.consider(CHSV(colour.h, 128, 255));
  //FastLED.delay(50);  //for the faries

  //paletteBeat( colour );
  //wave(colour);
  chase.consider(colour);  
  chase2.consider(colour);
  //chase3.consider(colour);
  //FastLED.delay(50);  //for the chase
  
  FastLED.show();
  
  
  
  
}



static void paletteBeat(CHSV c) {

    CRGBPalette16 currentPalette = 
                                   CRGBPalette16(
                                   CHSV(c.h, c.s, WASH),  CHSV(c.h, c.s, 128),  CHSV(c.h, c.s, 200),  CHSV(c.h, c.s, 128),
                                   CHSV(c.h, c.s, 255),   CHSV(c.h, c.s, 255),  CHSV(c.h, c.s, 255),  CHSV(c.h, c.s, 128),
                                   CHSV(c.h, c.s, WASH),  CHSV(c.h, c.s, WASH),  CHSV(c.h, c.s, WASH),   CHSV(c.h, c.s, WASH),
                                   CHSV(c.h, c.s, WASH),  CHSV(c.h, c.s, WASH),  CHSV(c.h, c.s, WASH),   CHSV(c.h, c.s, WASH) ); 

   static uint8_t phase = 0;
   
   CRGB theColor = ColorFromPalette(currentPalette, phase, 255, LINEARBLEND);   
   Serial.print("paletteBeat: ");
   Serial.print(theColor.r);
   Serial.print(" ");
   Serial.print(theColor.g);
   Serial.print(" ");
   Serial.println(theColor.b);

   fill_solid( leds, NUM_LEDS, theColor );
   phase++;
}

static void beat(CHSV c) {

  //So I'm not sue how one should un-FFT a wave, but here goes:
  
  //uint16_t w1 = beatsin16(60, 0, 66535, 0, 0,  );
  //uint16_t w2 = beatsin16(180, 0, 66535, 0, 0,  );
  
  fill_solid( leds, NUM_LEDS, CHSV( c.h, c.s, beat8( 80 ) ) );
}

static void wave( CHSV c ) {

  static int phase;  
  static int8_t inc = 1;
  
  for(uint8_t j = 0; j<NUM_LEDS;j++) {
    leds[j] |= CHSV(c.h, c.s,  beatsin8( 120, WASH, 255, map(j, 0, NUM_LEDS, 0, 255*5) ));
  }  

  switch( random(0, NUM_LEDS * 3) )
  {
    case 0:
      inc = 1;
      break;
    case 1:
      inc = -1;
      break;
    case 2:
      inc = 2;
      break;
    case 3:
      inc = -2;
      break;
  }
  phase+=inc; 
  
}



static uint8_t canRead() {
  return( ble.available() );
}

static CRGB optionallyReadColour(CRGB c) {
  //if(ble.available()) {
  if(true) {
    uint8_t len = readPacket(&ble, BLE_READPACKET_TIMEOUT);
    if (len == 0) return c;
    printHex(packetbuffer, len);
    // Color
    if (packetbuffer[1] == 'C') {
      uint8_t red = packetbuffer[2];
      uint8_t green = packetbuffer[3];
      uint8_t blue = packetbuffer[4];
      CRGB out = CRGB(red,green,blue);
      return(out);
    } else {
      return c;     
    }
  }
  return c;  
}



