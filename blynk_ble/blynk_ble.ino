#define BLYNK_DEBUG
#define BLYNK_PRINT Serial
#define BLYNK_USE_DIRECT_CONNECT

#include <SoftwareSerial.h>
#include <BlynkSimpleSerialBLE.h>
#include <SPI.h>

#include <Adafruit_BLE.h>
#include <Adafruit_BluefruitLE_SPI.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "d5a3662624fb4c0c917559e29a6c6888";
            //"3e9f859dff214afaa167c250c0ac73c6";

// SHARED SPI SETTINGS
// ----------------------------------------------------------------------------------------------
// The following macros declare the pins to use for HW and SW SPI communication.
// SCK, MISO and MOSI should be connected to the HW SPI pins on the Uno when
// using HW SPI.  This should be used with nRF51822 based Bluefruit LE modules
// that use SPI (Bluefruit LE SPI Friend).
// ----------------------------------------------------------------------------------------------
#define BLUEFRUIT_SPI_CS               8
#define BLUEFRUIT_SPI_IRQ              7
#define BLUEFRUIT_SPI_RST              4    // Optional but recommended, set to -1 if unused

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

void setup() {
  delay(1500);
  Serial.begin(9600);
  Serial.println(F("MSH Blynk BLE Yay")); 
  
  Blynk.begin(auth, ble);
  ble.begin(true);  
  ble.factoryReset(); //Optional
  ble.setMode(BLUEFRUIT_MODE_DATA);  
}

void loop() {  
  if (true) {    // If BLE is connected...    
    Blynk.run();    
  }
}
