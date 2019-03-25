#ifndef _MCP4251_H_
#define _MCP4251_H_

#include <SPI.h>

class MCP4251 {

private:
  const uint8_t AddressWiper0 = B00000000;
  const uint8_t AddressWiper1 = B00010000;
  const uint8_t AddressTCON = B01000000;
  const uint8_t AddressStatus = B01010000;

  const uint8_t csPin;

public:

  MCP4251(uint8_t csPin) : csPin(csPin) {
  }

  void setWiper0(uint8_t value);
  void setWiper1(uint8_t value);

private:

  void digitalPotWrite(uint8_t address, uint8_t value);

};

#endif // _MCP4251_H_
