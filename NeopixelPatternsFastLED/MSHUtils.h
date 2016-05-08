#include <avr/pgmspace.h>
 
int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

// Draw an "Integer Bar" of light starting at pixel 'pos', with given
// width (in whole pixels) and hue.
// This is not the interesting code.
void drawIntegerBar( CRGB* leds, uint8_t NUM_LEDS, uint8_t intpos, uint8_t width, CHSV c)
{
  int i = intpos; // start drawing at "I"
  for( int n = 0; n < width; n++) {
    leds[i] += CHSV( c.hue, c.sat, 255);
    i++;
    if( i == NUM_LEDS) i = 0; // wrap around
  }
}
 
 
// Draw a "Fractional Bar" of light starting at position 'pos16', which is counted in
// sixteenths of a pixel from the start of the strip.  Fractional positions are
// rendered using 'anti-aliasing' of pixel brightness.
// The bar width is specified in whole pixels.
// Arguably, this is the interesting code.
void drawFractionalBar( CRGB* leds, uint8_t NUM_LEDS, int pos16, uint8_t width, CHSV c)
{
  int i = pos16 / 16; // convert from pos to raw pixel number
  uint8_t frac = pos16 & 0x0F; // extract the 'factional' part of the position
 
  // brightness of the first pixel in the bar is 1.0 - (fractional part of position)
  // e.g., if the light bar starts drawing at pixel "57.9", then
  // pixel #57 should only be lit at 10% brightness, because only 1/10th of it
  // is "in" the light bar:
  //
  //                       57.9 . . . . . . . . . . . . . . . . . 61.9
  //                        v                                      v
  //  ---+---56----+---57----+---58----+---59----+---60----+---61----+---62---->
  //     |         |        X|XXXXXXXXX|XXXXXXXXX|XXXXXXXXX|XXXXXXXX |  
  //  ---+---------+---------+---------+---------+---------+---------+--------->
  //                   10%       100%      100%      100%      90%        
  //
  // the fraction we get is in 16ths and needs to be converted to 256ths,
  // so we multiply by 16.  We subtract from 255 because we want a high
  // fraction (e.g. 0.9) to turn into a low brightness (e.g. 0.1)
  uint8_t firstpixelbrightness = 255 - (frac * 16);
 
  // if the bar is of integer length, the last pixel's brightness is the
  // reverse of the first pixel's; see illustration above.
  uint8_t lastpixelbrightness  = 255 - firstpixelbrightness;
 
  // For a bar of width "N", the code has to consider "N+1" pixel positions,
  // which is why the "<= width" below instead of "< width".
 
  uint8_t bright;
  for( int n = 0; n <= width; n++) {
    if( n == 0) {
      // first pixel in the bar
      bright = firstpixelbrightness;
    } else if( n == width ) {
      // last pixel in the bar
      bright = lastpixelbrightness;
    } else {
      // middle pixels
      bright = 255;
    }
   
    leds[i] += CHSV( c.hue, c.sat, bright);
    i++;
    if( i == NUM_LEDS) i = 0; // wrap around
  }
}
