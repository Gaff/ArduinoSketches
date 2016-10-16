/*
#define BLYNK_DEBUG
#define BLYNK_PRINT Serial

#define BLYNK_USE_DIRECT_CONNECT

#include <SoftwareSerial.h>
#include <BlynkSimpleSerialBLE.h>
#include <SPI.h>
//#include <BLEPeripheral.h>
//#include "BLESerial.h"

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "YourAuthToken";

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

void setup() {
  delay(1500);
  Serial.begin(9600);
  Serial.println(F("MSH Blynk BLE Yay"));

  ble.begin(VERBOSE_MODE);
  ble.setMode(BLUEFRUIT_MODE_DATA);



  Blynk.begin(auth, Serial);
}

void loop() {
  //SerialBLE.poll();
  Serial.println(F("BT connected"));

  if (true) {    // If BLE is connected...    
    Blynk.run();
  }
}
*/
