#include "BluefruitSerial.h"
#define FACTORYRESET_ENABLE true
#define VERBOSE_MODE true

void error(const __FlashStringHelper *err) {
  Serial.println(err);
  while (1);
}

BluefruitSerial::BluefruitSerial(unsigned char cs, unsigned char irq, unsigned char rst)
  : _ble(cs, irq, rst)
{ 
}

void BluefruitSerial::begin(...) {
  _ble.begin(VERBOSE_MODE);
  
  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset."));
    if ( ! this->_ble.factoryReset() ){
      error(F("Couldn't factory reset"));
    }
  }

  _ble.setMode(BLUEFRUIT_MODE_DATA);
  //_ble.verbose(false); //My goodness this is noisy!
  /* Wait for connection */
  Serial.println(F("Waiting for connection"));
  while (! this->_ble.isConnected()) {
      delay(500);
  }   
  Serial.println(F("Connected"));
}

void BluefruitSerial::poll() {
}

//The good news is that _ble is a Stream. Actually we could just inherit Adafruit_BLE
int BluefruitSerial::available() {
  return _ble.available();
}
int BluefruitSerial::peek(void) {
  return _ble.peek();
};
int BluefruitSerial::read(void) {  
  int v = _ble.read();
  Serial.print("<-: ");
  Serial.println(v);
  return v;
};
void BluefruitSerial::flush(void) {
  return _ble.flush();
};

//Adafruit also gives us a print interface; woot.
size_t BluefruitSerial::write(uint8_t byte) {
  Serial.print("->: ");
  Serial.println(byte);
  return _ble.write(byte);
}
size_t BluefruitSerial::write(const uint8_t *buff, size_t len) {
  Serial.print("->: ");
  Serial.write(buff, len);
  Serial.println();  
  return _ble.write(buff, len);
}

BluefruitSerial::operator bool() {
  bool retval = _ble.isConnected();
  return retval;
}

