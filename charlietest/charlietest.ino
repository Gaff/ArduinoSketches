#include <Wire.h>           // For I2C communication
#define LEFT_I2C_ADDR 0x74       // I2C address of Charlieplex matrix
#define RIGHT_I2C_ADDR 0x75       // I2C address of Charlieplex matrix

#define WIDTH 16
#define HEIGHT 9

uint8_t        page = 0;    // Front/back buffer control
uint8_t        img_buff[WIDTH * HEIGHT]; // Buffer for rendering image

const uint8_t PROGMEM eye[144] = {
  0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
  0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X27,0X27,0X06,0X00,0X00,
  0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X06,0X00,0X81,0X27,0X00,0X00,
  0X00,0X00,0X00,0X00,0X00,0X06,0X27,0X00,0X00,0X00,0X27,0X00,0XFF,0XFF,0X27,0X00,
  0X00,0X00,0X00,0X06,0XFF,0X27,0X81,0X06,0X06,0X06,0X81,0X06,0XFF,0XFF,0X27,0X00,
  0X00,0X00,0X27,0XFF,0XFF,0X81,0X27,0X81,0X81,0X81,0X27,0X81,0XFF,0XFF,0X06,0X00,
  0X00,0X06,0X81,0X81,0XFF,0XFF,0X81,0X27,0X27,0X27,0X81,0XFF,0XFF,0X27,0X00,0X00,
  0X00,0X00,0X00,0X06,0X06,0X27,0X81,0X81,0XFF,0XFF,0XFF,0X81,0X06,0X00,0X00,0X00,
  0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
};

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

void readframe(const uint8_t *ptr, uint8_t *img) {  
  for (uint8_t x = 0; x < WIDTH * HEIGHT; x++ ) {
    img[x] = pgm_read_byte(ptr++);
  }
}

void inverse(uint8_t *img) { 
  for (uint8_t x = 0; x < WIDTH * HEIGHT; x++ ) {    
    img[x] = 255 - img[x];    
  }
}

void writeframe(const uint8_t addr, uint8_t page, uint8_t *img) {  
  // Write img[] to matrix (not actually displayed until next pass)
  pageSelect(addr, page);    // Select background buffer
  writeRegister(addr, 0x24); // First byte of PWM data
  uint8_t i = 0, byteCounter = 1;
  for(uint8_t x=0; x<HEIGHT; x++) {
    for(uint8_t y=0; y<WIDTH; y++) {
      Wire.write(img[i++]);      // Write each byte to matrix
      if(++byteCounter >= 32) {  // Every 32 bytes...
        byteCounter = 1;
        Wire.endTransmission();  // end transmission and
        writeRegister(addr, 0x24 + i); // start a new one (Wire lib limits)
      }
    }
  }
  Wire.endTransmission();
}

void writecolour(const uint8_t addr, uint8_t page, uint8_t color) {    
  pageSelect(addr, page);    // Select background buffer
  writeRegister(addr, 0x24); // First byte of PWM data
  uint8_t i = 0, byteCounter = 1;
  for(uint8_t x=0; x<HEIGHT; x++) {
    for(uint8_t y=0; y<WIDTH; y++) {
      Wire.write(color);      // Write each byte to matrix
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


void clearScreen(const uint8_t addr) { //Really this is a reset!
  uint8_t i, p, byteCounter;
  
  pageSelect(addr, 0x0B);                        // Access the Function Registers
  writeRegister(addr, 0);                        // Starting from first...
  for(i=0; i<13; i++) 
    Wire.write(10 == i); // Clear all except Shutdown
  Wire.endTransmission();
  for(p=0; p<8; p++) {                     // For each page used (0 & 1)...
    pageSelect(addr, p);                         // Access the Frame Registers
    writeRegister(addr, 0);                      // Start from 1st LED control reg
    for(i=0; i<18; i++) 
      Wire.write(0xFF);  // Enable all LEDs (18*8=144)  
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


void setup() {
  delay(1000);
  Serial.begin(57600);
  Serial.println("Flame Pendant - test");

  pinMode(0, INPUT);
  pinMode(1, INPUT);

  Wire.begin();                            // Initialize I2C
  clearScreen(LEFT_I2C_ADDR);
  clearScreen(RIGHT_I2C_ADDR);

  readframe(&eye[0], &img_buff[0]);
  writecolour(LEFT_I2C_ADDR, 0, 0xFF);
  writecolour(RIGHT_I2C_ADDR, 0, 0xFF);      
  writecolour(LEFT_I2C_ADDR, 1, 0);
  writecolour(RIGHT_I2C_ADDR, 1, 0);
  writecolour(LEFT_I2C_ADDR, 2, 20);
  writecolour(RIGHT_I2C_ADDR, 2, 20);      

  writeframe(LEFT_I2C_ADDR, 7, &img_buff[0]);
  writeframe(RIGHT_I2C_ADDR, 7, &img_buff[0]);  
  inverse(&img_buff[0]);
  writeframe(LEFT_I2C_ADDR, 6, &img_buff[0]);
  writeframe(RIGHT_I2C_ADDR, 6, &img_buff[0]);  
}

void loop() {
  page++;
  if(page >= 2) page = 0;

  Serial.println(page);

  if(digitalRead(0) == HIGH) {  
    displayPage(LEFT_I2C_ADDR, 7);
  } else {
    displayPage(LEFT_I2C_ADDR, page);  
  }
  if(digitalRead(1) == HIGH) {  
    displayPage(RIGHT_I2C_ADDR, 7);
  } else {
    displayPage(RIGHT_I2C_ADDR, page);  
  }

  delay(3000);
}
