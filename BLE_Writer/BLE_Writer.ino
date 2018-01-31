
//Leave these at the top, becase C++.
#define BLYNK_DEBUG
#define BLYNK_DEBUG_ALL
#define BLYNK_PRINT Serial
#define BLYNK_USE_DIRECT_CONNECT

//Make the performance suck
//#define BLYNK_MSG_LIMIT 5

#define DBG_ENABLE 1

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
char auth[] = "570529042ad34219a964929e2a483f40";

//===============
// BLE setup
//===============

// SHARED SPI SETTINGS (see adafruit webpages for details)
#define BLUEFRUIT_SPI_CS               8
#define BLUEFRUIT_SPI_IRQ              7
#define BLUEFRUIT_SPI_RST              4    // Optional but recommended, set to -1 if unused
#define BLUEFRUIT_VERBOSE_MODE         true
#define BLYNK_MSG_LIMIT                5
#define BLE_SERIAL_DEBUG               true

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);


#define INTERVAL 3000

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
  ble.println("ATE=1");
  // Check response status
  ble.waitForOK();  
  ble.setMode(BLUEFRUIT_MODE_DATA);   
  
  unsigned long currentMillis = millis();  //0; //force an initial publish
  previousMillis = currentMillis;
  previousPollMillis = currentMillis;

  Blynk.begin(ble, auth);
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

BLYNK_WRITE(V9) //psuedo echo
{
  Blynk.virtualWrite(V8, param.asInt());
}

void loop() {

  Blynk.run();
  
  unsigned long currentMillis = millis();    
  if(currentMillis - previousMillis > INTERVAL) {
    previousMillis = currentMillis;   
    dowrites();    
  }  
}

void dowrites() {
  Serial.println(F("Tick"));
  thevalue++;
  
  Blynk.virtualWrite(V0, thevalue);
  Blynk.virtualWrite(V1, thevalue);  
  Blynk.virtualWrite(V2, thevalue);  
  Blynk.virtualWrite(V3, thevalue); 
  
  //ble.flush(); 
}

