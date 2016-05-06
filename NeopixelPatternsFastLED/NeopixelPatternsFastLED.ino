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

#define NUM_LEDS 92
#define LED_PIN 11
#define COLOR_ORDER GRB
#define CHIPSET     WS2811
CRGB leds[NUM_LEDS];

#define WASH 62 //should be something light 40/brightness

void error(const __FlashStringHelper *err) {
  Serial.println(err);
  while (1);
}

template <uint8_t Tpoints, uint8_t Tframes_to_next = 4, uint8_t Tframes_to_max = 16>
class CFaries {  
  uint8_t m_points[Tpoints];   
  uint16_t sequence = 0; 
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
        uint16_t seqi = sequence; //how long i has been alive.
        seqi = (sequence + i*Tframes_to_next) % (Tframes_to_next*Tpoints);
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
    
      sequence++;
      if( sequence == Tframes_to_next*Tpoints )
        sequence = 0;      
    }
};

CFaries<28> faries;
CFaries<4, 4, 8> sparkles;


void setup() {
  delay(1500);
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  delay(3000); // sanity delay
  

  Serial.begin(115200);
  
  Serial.println(F("MSH Neopixels FASTLED Yay"));
  Serial.println(F("-----------------------------------------"));
  
  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  
  ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
  ble.setMode(BLUEFRUIT_MODE_DATA);

  Serial.println(F("Bluefruit setup"));
  Serial.println(F("-----------------------------------------"));

  memset(leds, 0, sizeof( leds) );
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(80);
  //coming in the next version!
  //FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
  FastLED.show();    

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
  Serial.print("Loop: ");
  Serial.println(micros());

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
  
  chase(colour);    
  FastLED.show();
  
  //FastLED.delay(10);  
  
}



//Note that the strip is 2xWIDTH wide, half faded in, half faded out.
#define WIDTH 16
static void chase(CHSV c) {  

  static uint8_t i;  
  static int8_t inc = 1;

  uint16_t j;
  
  for(j = 0; j<WIDTH; j++ ) {        
    leds[Pos(i+j)] |= CHSV(c.h, c.s, map(j, 0, WIDTH, WASH, 255) );
  }
  for(j = 0; j<WIDTH; j++ ) {    
    leds[Pos(i+j+WIDTH)] |= CHSV(c.h, c.s, map(j, 0, WIDTH, 255, WASH) );
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
  i=Pos(i+inc);    
}

static uint16_t Pos(uint16_t raw) {
  return (NUM_LEDS + raw) % NUM_LEDS;
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



