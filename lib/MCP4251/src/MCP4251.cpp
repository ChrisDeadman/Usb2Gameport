#include "MCP4251.h"

void MCP4251::setWiper0(uint8_t value) {
  digitalPotWrite(AddressWiper0, value);
}

void MCP4251::setWiper1(uint8_t value) {
  digitalPotWrite(AddressWiper1, value);
}

inline void MCP4251::digitalPotWrite(uint8_t address, uint8_t value) {
  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  digitalWrite(csPin, LOW);
  SPI.transfer(address);
  SPI.transfer(value);
  digitalWrite(csPin, HIGH);
  SPI.endTransaction();
}
