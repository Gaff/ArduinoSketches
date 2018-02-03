//--------------------------------------------------------------------------
// Animated flame for Adafruit Pro Trinket.  Uses the following parts:
//   - Pro Trinket microcontroller (adafruit.com/product/2010 or 2000)
//     (#2010 = 3V/12MHz for longest battery life, but 5V/16MHz works OK)
//   - Charlieplex LED Matrix Driver (2946)
//   - Charlieplex LED Matrix (2947, 2948, 2972, 2973 or 2974)
//   - 350 mAh LiPoly battery (2750)
//   - LiPoly backpack (2124)
//   - SPDT Slide Switch (805)
//
// This is NOT good "learn from" code for the IS31FL3731; it is "squeeze
// every last byte from the Pro Trinket" code.  If you're starting out,
// download the Adafruit_IS31FL3731 and Adafruit_GFX libraries, which
// provide functions for drawing pixels, lines, etc.  This sketch also
// uses some ATmega-specific tricks and will not run as-is on other chips.
//--------------------------------------------------------------------------

#include <Wire.h>           // For I2C communication
#include "eyesdata.h"           // Flame animation data
//#include <avr/power.h>      // Peripheral control and
//#include <avr/sleep.h>      // sleep to minimize current draw
//#include "firependant.h"

#define LEFT_I2C_ADDR 0x74       // I2C address of Charlieplex matrix
#define RIGHT_I2C_ADDR 0x75       // I2C address of Charlieplex matrix

#define WIDTH 16
#define HEIGHT 9

uint8_t     global_brightness = 128;

typedef struct imgarray_t { uint8_t d[WIDTH * HEIGHT]; } imgarray_t;
typedef struct ImageLoad{
  uint8_t frame;
  bool flip;  
  uint8_t blinkframe;

  ImageLoad(int frame, bool flip):frame(frame),flip(flip),blinkframe(0){};
};

uint8_t        page = 0;    // Front/back buffer control
//It's possible with some functional magic to do away with this buffer, but life is much simpler with it.
imgarray_t     img; // Buffer for rendering image

// UTILITY FUNCTIONS -------------------------------------------------------

// The full IS31FL3731 library is NOT used by this code.  Instead, 'raw'
// writes are made to the matrix driver.  This is to maximize the space
// available for animation data.  Use the Adafruit_IS31FL3731 and
// Adafruit_GFX libraries if you need to do actual graphics stuff.

// Begin I2C transmission and write register address (data then follows)
void writeRegister(const uint8_t addr, uint8_t n) {
  Wire.beginTransmission(addr);
  Wire.write(n);
  // Transmission is left open for additional writes
}

// Select one of eight IS31FL3731 pages, or Function Registers
void pageSelect(uint8_t addr, uint8_t n) {
  writeRegister(addr, 0xFD); // Command Register
  Wire.write(n);       // Page number (or 0xB = Function Registers)
  Wire.endTransmission();
}

void loadframe_raw(uint8_t frame, imgarray_t &img, bool flip) {

  const uint8_t *ptr  = frames[frame];

  for(uint8_t x=0; x<HEIGHT; x++) { 
    int offset = x*WIDTH;   
    for(uint8_t y=0; y<WIDTH; y++) {
      int yoffset;
      if(flip) {
        yoffset = y;
      } else {
        yoffset = WIDTH-y;
      }
      img.d[offset+y] = pgm_read_byte(ptr+offset+yoffset);     
    }
  }
}

void apply_brightness(imgarray_t &img) {
  //Copied from Adafruit's neopixel library - I'm that lazy!
  if(global_brightness == 0)
    return;
  
  for(uint8_t x=0; x<HEIGHT*WIDTH; x++) {
    img.d[x] = (img.d[x] * global_brightness) >> 8;
  }
}

void loadframe(ImageLoad &l, imgarray_t &img) {
    loadframe_raw(l.frame, img, l.flip);
    apply_brightness(img);
}

void writeframe(const uint8_t addr, uint8_t page, imgarray_t &img) {
  
  // Write img[] to matrix (not actually displayed until next pass)
  pageSelect(addr, page);    // Select background buffer
  writeRegister(addr, 0x24); // First byte of PWM data
  uint8_t i = 0, byteCounter = 1;
  for(uint8_t x=0; x<9; x++) {
    for(uint8_t y=0; y<16; y++) {
      Wire.write(img.d[i]);      // Write each byte to matrix
      i++;
      if(++byteCounter >= 32) {  // Every 32 bytes...
        byteCounter = 1;
        Wire.endTransmission();  // end transmission and
        writeRegister(addr, 0x24 + i); // start a new one (Wire lib limits)
      }
    }
  }
  Wire.endTransmission();
}

void clearScreen(const uint8_t addr) {
  uint8_t i, p, byteCounter;
  
  pageSelect(addr, 0x0B);                        // Access the Function Registers
  writeRegister(addr, 0);                        // Starting from first...
  for(i=0; i<13; i++) Wire.write(10 == i); // Clear all except Shutdown
  Wire.endTransmission();
  for(p=0; p<8; p++) {                     // For each page used (0 & 1)...
    pageSelect(addr, p);                         // Access the Frame Registers
    writeRegister(addr, 0);                      // Start from 1st LED control reg
    for(i=0; i<18; i++) Wire.write(0xFF);  // Enable all LEDs (18*8=144)    
    for(byteCounter = i+1; i<0xB4; i++) {  // For blink & PWM registers...
      Wire.write(0);                       // Clear all
      if(++byteCounter >= 32) {            // Every 32 bytes...
        byteCounter = 1;                   // End I2C transmission and
        Wire.endTransmission();            // start a new one because
        writeRegister(addr, i);                  // Wire buf is only 32 bytes.
      }
    }
    Wire.endTransmission();
  }
}

void displayPage(const uint8_t addr, uint8_t page) {
  pageSelect(addr, 0x0B);    // Function registers
  writeRegister(addr, 0x01); // Picture Display reg
  Wire.write(page);    // Page #
  Wire.endTransmission();  

}

// SETUP FUNCTION - RUNS ONCE AT STARTUP -----------------------------------

void setup() {
  //uint8_t i, p, byteCounter;

  delay(1000);
  Serial.begin(57600);
  Serial.println("Flame Pendant");

  //power_all_disable(); // Stop peripherals: ADC, timers, etc. to save power
  //power_twi_enable();  // But switch I2C back on; need it for display
  //DIDR0 = 0x0F;        // Digital input disable on A0-A3

  // The Arduino Wire library runs I2C at 100 KHz by default.
  // IS31FL3731 can run at 400 KHz.  To ensure fast animation,
  // override the I2C speed settings after init...
  Wire.begin();                            // Initialize I2C
  //Wire.setClock();                  // Go Faster!
  //Wire.begin(D1, D2);
  //TWSR = 0;                                // I2C prescaler = 1
  //TWBR = (F_CPU / 400000 - 16) / 2;        // 400 KHz I2C
  // The TWSR/TWBR lines are AVR-specific and won't work on other MCUs.

  clearScreen(LEFT_I2C_ADDR);
  clearScreen(RIGHT_I2C_ADDR);

  ImageLoad il(4, false);
  loadframe(il, img);
  writeframe(LEFT_I2C_ADDR, 1, img);  
  il.flip = true;
  loadframe(il, img);
  writeframe(RIGHT_I2C_ADDR, 1, img);  
  
  displayPage(LEFT_I2C_ADDR, 1);
  displayPage(RIGHT_I2C_ADDR, 1);
}

//Eye moving center > in: 4 > 3 > 1 > 0
//Eye moving in > center: 0 > 1 > 2 > 4
//Eye moving center > out: 4 > 5 > 6 > 8
//Eye moving out > center: 8 > 7 > 6 > 4
uint8_t centre_in_centre[] = {4,3,1,0,0,1,2,4};
uint8_t centre_out_centre[] = {4,5,6,8,8,7,6,4};


void loadFramesLR(uint8_t left[], uint8_t right[]) {  
  ImageLoad il(4, false);
  for(int i = 0; i < 8; i++ ) {  
    il.frame = left[i];
    loadframe(il, img);
    writeframe(LEFT_I2C_ADDR, i, img);    
  }
  il.flip = true;
  for(int i =0; i < 8; i++) {
    il.frame = right[i];
    loadframe(il, img);
    writeframe(RIGHT_I2C_ADDR, i, img);            
  }
}


void loadFrames() {  
  int anim = random(2);
  Serial.print(F("Loading animation "));  
  Serial.println(anim);  
  switch(anim) {
    case 0: //left
      loadFramesLR(centre_in_centre, centre_out_centre);
      break;      
    case 1: //right
      loadFramesLR(centre_out_centre, centre_in_centre);
      break;
    case 2: //stare
      loadFramesLR(centre_in_centre, centre_in_centre);      
      break;
  }
}


//void loop() {}
// LOOP FUNCTION - RUNS EVERY FRAME ----------------------------------------


uint8_t framep = 0;
unsigned long fc = 0;

void loop() {

  if(framep == 0) {
    loadFrames();
    delay(2000);
  }  

  displayPage(LEFT_I2C_ADDR, framep);
  displayPage(RIGHT_I2C_ADDR, framep);
  delay(50);  

  framep++;
  if(framep == 4)
    delay(150);
  if(framep > 7) framep = 0;

}

  /*
void loop() {
  uint8_t frame = 0;
  frame = centre_in_centre[framep];
  framep++;
  if(framep / sizeof(frameorder)) framep = 0;
  
  //power_twi_enable();
  // Datasheet recommends that I2C should be re-initialized after enable,
  // but Wire.begin() is slow.  Seems to work OK without.  

  // Display frame rendered on prior pass.  This is done at function start
  // (rather than after rendering) to ensire more uniform animation timing.
  displayPage(LEFT_I2C_ADDR, page);
  displayPage(RIGHT_I2C_ADDR, page);
  
  if(fc % 10 == 0 )
    Serial.println(millis());
  fc++;

  //delay(250);
  
  page ^= 1; // Flip front/back buffer index  


  DirectReadImage img(frame);  
  writeframe(RIGHT_I2C_ADDR, page, img);
  img.flip = true;
  writeframe(LEFT_I2C_ADDR, page, img);  


  //power_twi_disable(); // I2C off (see comment at top of function)
  //sleep_enable();
  //interrupts();
  //sleep_mode();        // Power-down MCU.
  // Code will resume here on wake; loop() returns and is called again

}*/


//ISR(WDT_vect) { } // Watchdog timer interrupt (does nothing, but required)
