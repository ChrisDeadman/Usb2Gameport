#ifndef HID_MOUSE_CONTROLLER_H
#define HID_MOUSE_CONTROLLER_H

#include <hiduniversal.h>
#include "HIDJoystickState.h"
#include "HIDDescriptionParser.h"

class HIDJoystickController : HIDUniversal {

private:

  USBHost * const usb;
  HIDDescriptionParser hidParser;
  HIDJoystickState state;

public:

  HIDJoystickController(USBHost *usb) : HIDUniversal::HIDUniversal(usb), usb(usb), hidParser(usb) {
  };

  bool isConnected();

  HIDJoystickState * getState();

  uint32_t Init(uint32_t parent, uint32_t port, uint32_t lowspeed) override;

  void ParseHIDData(HID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf) override;
};

#endif // HID_MOUSE_CONTROLLER_H
