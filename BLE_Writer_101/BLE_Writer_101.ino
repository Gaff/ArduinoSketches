
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
char auth[] = "570529042ad34219a964929e2a483f40";


BLEPeripheral  blePeripheral;

#define INTERVAL 1000
#define POLL_INTERVAL 100
int thevalue;
long previousMillis;
long previousPollMillis;

void setup()
{
  // Debug console
  Serial.begin(9600);

  delay(1000);

  blePeripheral.setLocalName("Blynk101");
  blePeripheral.setDeviceName("Blynk101");
  blePeripheral.setAppearance(384);

  Blynk.begin(blePeripheral, auth);

  blePeripheral.begin();

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

  blePeripheral.poll();  
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

