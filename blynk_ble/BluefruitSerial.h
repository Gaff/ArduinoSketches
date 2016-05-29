#ifndef _BLUEFRUIT_SERIAL_H_
#define _BLUEFRUIT_SERIAL_H_

#include <Arduino.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

class BluefruitSerial : public Stream
{
  public:
    BluefruitSerial(unsigned char cs, unsigned char irq, unsigned char rst);

    void begin(...);
    void poll();
    void end();

    //AFAICT This is the interface we need to implement in order to get blynk to work.

    //From Arduino Stream.h:
    virtual int available(void);
    virtual int peek(void);
    virtual int read(void);
    virtual void flush(void);

    //Writey stuff:
    virtual size_t write(uint8_t byte);
    virtual size_t write(const uint8_t *buff, size_t len);
    using Print::write; //This gives us the Print::write method in our scope [I think!]
    virtual operator bool(); //allows is to overload if(x)

  private:
    Adafruit_BluefruitLE_SPI _ble;
    
};

#endif

