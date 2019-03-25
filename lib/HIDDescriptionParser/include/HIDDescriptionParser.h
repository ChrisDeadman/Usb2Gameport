#ifndef _HID_DESCRIPTION_PARSER_H_
#define _HID_DESCRIPTION_PARSER_H_

#include <hiduniversal.h>

#define HID_JOYSTICK_DEBUG(x) Serial1.x
//#define HID_JOYSTICK_DEBUG(x)

#define HID_DESCRIPTOR_REPORT 0x22
#define USB_CLASS_CODE_HID 0x03

struct HIDReportUsageDescription {
  uint8_t usagePage;
  uint8_t usage;
  uint16_t logicalMinimum;
  uint16_t logicalMaximum;
  uint16_t physicalMinimum;
  uint16_t physicalMaximum;
  uint8_t usageMinimum;
  uint8_t usageMaximum;
  uint8_t reportSize;
  uint8_t reportCount;
  uint8_t placeholder;
};

class HIDDescriptionParser {

private:

  static const int MAX_USAGE_DESCRIPTIONS = 10;

  USBHost * const usb;

  HIDReportUsageDescription usageDescriptions[MAX_USAGE_DESCRIPTIONS];
  int usageDescriptionCount = 0;

public:

  HIDDescriptionParser(USBHost *usb) : usb(usb) {
  };

  int getUsageDescriptionCount();
  HIDReportUsageDescription * getUsageDescription(int idx);

  void parseHidDataBlock(HIDReportUsageDescription * description, uint8_t * input, int * inputIdx, uint8_t * bitIdx, uint8_t * output, int * outputSize);
  void parseDeviceDescriptor(uint8_t address, uint8_t endpoint);
};

#endif // _HID_DESCRIPTION_PARSER_H_
