
//Leave these at the top, becase C++.
#define BLYNK_DEBUG
#define BLYNK_PRINT Serial
//#define BLYNK_USE_DIRECT_CONNECT

#include <BlynkSimpleCurieBLE.h>
#include <CurieBLE.h>

//===============
// Blynk setup
//===============

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "9aef6a9e386b49a98013b6fb364cc0db";

//===============
// BLE setup
//===============

BLEPeripheral  ble;

#define INTERVAL 5000
#define POLL_INTERVAL 1000
int thevalue;
long previousMillis;
long previousPollMillis;

void setup() {
  Serial.begin(9600);
  delay(1500);  
  Serial.println(F("Blynk for adafruit BLE modules with FAST LED neopixels")); 

  ble.setLocalName("Blynk101");
  ble.setDeviceName("Blynk101");
  ble.setAppearance(384);

  Blynk.begin(auth, ble);
  ble.begin();

  Serial.println("Waiting for connections...");
  
  previousMillis = 0; //force an initial publish
  previousPollMillis = 0;
}



void loop() {
  unsigned long currentMillis = millis();  
  if(currentMillis - previousMillis > INTERVAL) {
    previousMillis = currentMillis;   
    dowrites();    
  }

  Blynk.run();
  ble.poll();
}

void dowrites() {  
  thevalue++;
  Serial.println(F("Tick"));
  
  Blynk.virtualWrite(V0, thevalue);
  Blynk.virtualWrite(V1, thevalue);  
  Blynk.virtualWrite(V2, thevalue);  
  Blynk.virtualWrite(V3, thevalue); 

  //ble.flush(); 
}

/*
void pollBLE() {
  ble.setMode(BLUEFRUIT_MODE_COMMAND); 
  ble.println("AT+BLEUARTFIFO");
  ble.waitForOK();
  ble.setMode(BLUEFRUIT_MODE_DATA); 
}*/

