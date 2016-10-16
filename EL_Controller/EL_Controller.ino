#define ELPIN 1
#include <FastLED.h>

void setup() {
  delay(1500);
  pinMode(ELPIN, OUTPUT); 
}

void loop() {
  //uint8_t b = beatsin8(20);
  //uint8_t b2 = map(b, 0, 255, 127, 255);
  //analogWrite(ELPIN, b2);

  uint8_t b = beat8(120);
  if( b > 127 ) {
    analogWrite(ELPIN, 255);
  } else {
    analogWrite(ELPIN, 0);
  }
  

}
