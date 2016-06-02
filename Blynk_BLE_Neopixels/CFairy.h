#ifndef __CFairy_h__
#define __CFairy_h__

#include <FastLED.h>
#define RANGE_MAX 65535


template <uint8_t Tpoints>
class CFairy {    

  //uint8_t m_size;
  uint8_t m_pointdata[Tpoints];
  uint16_t m_priorbeat;
  static void renderpixel (CRGB* led, uint16_t b, CHSV c);
     
  public:
    //CFairy(uint8_t m_size, uint8_t num_leds);     
    CFairy(uint8_t num_leds);
    CFairy() : m_priorbeat(RANGE_MAX) {};
    void render(CRGB* leds, uint8_t num_leds, uint16_t beat, CHSV c);
    uint8_t getSize(); // {return Tpoints;} 
};


template <uint8_t Tpoints>
CFairy<Tpoints>::CFairy(uint8_t num_leds)
{
  for(uint8_t i = 0; i < Tpoints; i++ )
    m_pointdata[i] = random(0, num_leds);     
}

template <uint8_t Tpoints>
uint8_t CFairy<Tpoints>::getSize() {
  return Tpoints;
}

template <uint8_t Tpoints>
void CFairy<Tpoints>::render(CRGB* leds, uint8_t num_leds, uint16_t beat, CHSV c)
{    
    //Got to get through the whole sequence in 0..65535
    //consider LED 1, it rises for [say] 2/8, is high for 4/8, falls for 2/8    
    for(uint8_t i = 0; i < this->getSize(); i++ ) {
  
      
      uint16_t phase = 65535/this->getSize()*i;
      uint16_t b = (beat + phase) % RANGE_MAX;
      uint16_t pb = (m_priorbeat + phase) % RANGE_MAX;

      //Serial.print(i);
      //Serial.print(" - ");
      //Serial.println(b);
      
      if( b < pb ) {
        m_pointdata[i] = random(0, num_leds);
      }
      CFairy::renderpixel(&leds[m_pointdata[i]], b, c);
    }
    m_priorbeat = beat;
}

template <uint8_t Tpoints>
void CFairy<Tpoints>::renderpixel (CRGB* led, uint16_t b, CHSV c) 
{  
  CHSV dim;
  if( b < RANGE_MAX*2/8 ) {
      dim = CHSV(c.h, c.s, map(b, 0, RANGE_MAX*2/8, 0, 255 ) );      
      *led |= dim;
  } else if( b > RANGE_MAX*6/8 ) {
      dim = CHSV(c.h, c.s, map(b, RANGE_MAX*6/8, RANGE_MAX, 255, 0 ) );        
      *led |= dim;
  } else {      
      *led |= CHSV( c.h, c.s, 255);
  } 
}



#endif
