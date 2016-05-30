#include <SoftwareSerial.h>
#include <BlynkSimpleSerialBLE.h>
#include <SPI.h>

#include <Adafruit_BLE.h>
#include <Adafruit_BluefruitLE_SPI.h>

#define BLYNK_DEBUG
#define BLYNK_PRINT Serial
#define BLYNK_USE_DIRECT_CONNECT

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "d5a3662624fb4c0c917559e29a6c6888";            

// SHARED SPI SETTINGS (see adafruit webpages for details)
#define BLUEFRUIT_SPI_CS               8
#define BLUEFRUIT_SPI_IRQ              7
#define BLUEFRUIT_SPI_RST              4    // Optional but recommended, set to -1 if unused
#define BLUEFRUIT_VERBOSE_MODE         true

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

void setup() {
  Serial.begin(9600);
  Serial.println(F("Blynk for adafruit BLE modules")); 
  
  Blynk.begin(auth, ble);
  ble.begin(BLUEFRUIT_VERBOSE_MODE);   
  ble.factoryReset(); //Optional
  ble.setMode(BLUEFRUIT_MODE_DATA);  
}

void loop() {    
  Blynk.run();
}
