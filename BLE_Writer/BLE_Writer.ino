
//Leave these at the top, becase C++.
#define BLYNK_DEBUG
#define BLYNK_PRINT Serial
#define BLYNK_USE_DIRECT_CONNECT

#define DBG_ENABLE      1

#include <SoftwareSerial.h>

#include <SPI.h>
#include <Adafruit_BLE.h>
#include <Adafruit_BluefruitLE_SPI.h>


#include <BlynkSimpleSerialBLE.h>


//===============
// Blynk setup
//===============

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "9aef6a9e386b49a98013b6fb364cc0db";

//===============
// BLE setup
//===============

// SHARED SPI SETTINGS (see adafruit webpages for details)
#define BLUEFRUIT_SPI_CS               8
#define BLUEFRUIT_SPI_IRQ              7
#define BLUEFRUIT_SPI_RST              4    // Optional but recommended, set to -1 if unused
#define BLUEFRUIT_VERBOSE_MODE         true

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);


#define INTERVAL 5000
#define POLL_INTERVAL 1000
int thevalue;
long previousMillis;
long previousPollMillis;

void setup() {
  delay(1500);
  Serial.begin(115200);
  Serial.println(F("Dammint why doens't BLE work?"));   
  
  ble.begin(BLUEFRUIT_VERBOSE_MODE);   
  ble.factoryReset(); //Optional  
  ble.echo(true);
  ble.info();
  ble.setMode(BLUEFRUIT_MODE_DATA);   
  previousMillis = 0; //force an initial publish
  previousPollMillis = 0;

  Blynk.begin(auth, ble);
}

void pollBLE() {
  ble.setMode(BLUEFRUIT_MODE_COMMAND); 
  ble.println("AT+BLEUARTFIFO");
  ble.waitForOK();
  ble.setMode(BLUEFRUIT_MODE_DATA); 
}

BLYNK_WRITE(V10) //Random
{
  int pinData = param.asInt();
  Serial.print(F("Button 10  = "));
  Serial.println(pinData);
}

void loop() {
  unsigned long currentMillis = millis();  
  if(currentMillis - previousMillis > INTERVAL) {
    previousMillis = currentMillis;   
    dowrites();    
  }
  /*
  if(currentMillis - previousPollMillis > POLL_INTERVAL) {
    previousPollMillis = currentMillis;   
    pollBLE();    
  }  
  */
  Blynk.run();
}

void dowrites() {
  Serial.println(F("Tick"));
  thevalue++;
  
  Blynk.virtualWrite(V0, thevalue);
  Blynk.virtualWrite(V1, thevalue);  
  Blynk.virtualWrite(V2, thevalue);  
  Blynk.virtualWrite(V3, thevalue); 
  
  for(int i = 0; i < 4; i++)
  {
    ble.print("This is value: ");
    Serial.print("This is value: ");
    ble.println(thevalue);
    Serial.println( thevalue);
  }
  
  ble.flush(); 
}

