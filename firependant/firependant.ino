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
#include "eyes.h"           // Flame animation data
#include "eyes2.h"           // Flame animation data
//#include "data.h"
//#include <avr/power.h>      // Peripheral control and
//#include <avr/sleep.h>      // sleep to minimize current draw

#define LEFT_I2C_ADDR 0x74       // I2C address of Charlieplex matrix
#define RIGHT_I2C_ADDR 0x75       // I2C address of Charlieplex matrix

#define WIDTH 16
#define HEIGHT 9

uint8_t        page = 0;    // Front/back buffer control
const uint8_t *ptr  = anim; // Current pointer into animation data
uint8_t        img_buff[WIDTH * HEIGHT]; // Buffer for rendering image

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

void readframe(uint8_t frame, uint8_t *img) {

  const uint8_t *ptr  = frames[frame];
  
  for (uint8_t x = 0; x < WIDTH * HEIGHT; x++ ) {
    img[x] = pgm_read_byte(ptr++);
  }
  
    /*
  // Then render NEXT frame.  Start by getting bounding rect for new frame:
  a = pgm_read_byte(ptr++);     // New frame X1/Y1
  if(a >= 0x90) {               // EOD marker? (valid X1 never exceeds 8)
    ptr = anim;                 // Reset animation data pointer to start
    a   = pgm_read_byte(ptr++); // and take first value
  }
  x1 = a >> 4;                  // X1 = high 4 bits
  y1 = a & 0x0F;                // Y1 = low 4 bits
  a  = pgm_read_byte(ptr++);    // New frame X2/Y2
  x2 = a >> 4;                  // X2 = high 4 bits
  y2 = a & 0x0F;                // Y2 = low 4 bits

  // Read rectangle of data from anim[] into portion of img[] buffer
  for(x=x1; x<=x2; x++) { // Column-major
    for(y=y1; y<=y2; y++) img[(x << 4) + y] = pgm_read_byte(ptr++);
  }*/
}

void writeframe(const uint8_t addr, uint8_t page, uint8_t *img) {
  
  // Write img[] to matrix (not actually displayed until next pass)
  pageSelect(addr, page);    // Select background buffer
  writeRegister(addr, 0x24); // First byte of PWM data
  uint8_t i = 0, byteCounter = 1;
  for(uint8_t x=0; x<9; x++) {
    for(uint8_t y=0; y<16; y++) {
      Wire.write(img[i++]);      // Write each byte to matrix
      if(++byteCounter >= 32) {  // Every 32 bytes...
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

  readframe(4, &img_buff[0]);
  writeframe(LEFT_I2C_ADDR, 1, &img_buff[0]);
  writeframe(RIGHT_I2C_ADDR, 1, &img_buff[0]);
  
  //displayPage(LEFT_I2C_ADDR, 1);
  displayPage(RIGHT_I2C_ADDR, 1);
  delay(2000);
  displayPage(LEFT_I2C_ADDR, 1);
  
  // Enable the watchdog timer, set to a ~32 ms interval (about 31 Hz)
  // This provides a sufficiently steady time reference for animation,
  // allows timer/counter peripherals to remain off (for power saving)
  // and can power-down the chip after processing each frame.
  //set_sleep_mode(SLEEP_MODE_PWR_DOWN); // Deepest sleep mode (WDT wakes)
  //noInterrupts();
  //MCUSR  &= ~_BV(WDRF);
  //WDTCSR  =  _BV(WDCE) | _BV(WDE);     // WDT change enable
  //WDTCSR  =  _BV(WDIE) | _BV(WDP0);    // Interrupt enable, ~32 ms
  //interrupts();
  // Peripheral and sleep savings only amount to about 10 mA, but this
  // may provide nearly an extra hour of run time before battery depletes.
}

// LOOP FUNCTION - RUNS EVERY FRAME ----------------------------------------

uint8_t framep = 0;
bool dir = true;
unsigned long fc = 0;

//uint8_t frameorder[] = {0,1,2,3,4,5,6,7,8,8,8,8,8,8,7,6,5,4,3,2,1,0,0,0,0,0,0};
uint8_t frameorder[] = {4,4,4,4,5,6,8,8,8,8,7,6,4,4,4,4,4,4,4,4,4,4,4,4,3,1,0,0,0,0,1,2,4,4,4,4};

void loop() {
  
}
  
void loopx() {
  uint8_t frame = 0;
  frame = frameorder[framep];
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

  readframe(frame, &img_buff[0]);
  writeframe(LEFT_I2C_ADDR, page, &img_buff[0]);
  writeframe(RIGHT_I2C_ADDR, page, &img_buff[0]);




  //power_twi_disable(); // I2C off (see comment at top of function)
  //sleep_enable();
  //interrupts();
  //sleep_mode();        // Power-down MCU.
  // Code will resume here on wake; loop() returns and is called again

}



//ISR(WDT_vect) { } // Watchdog timer interrupt (does nothing, but required)
