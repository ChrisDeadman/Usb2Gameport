#include "HIDJoystickController.h"

// callbacks
extern "C" {
  void __hidJoystickStateChangeEmptyCallback(HIDJoystickState*) { }
}
// implement in your code
void hidJoystickStateChange(HIDJoystickState*) __attribute__ ((weak, alias("__hidJoystickStateChangeEmptyCallback")));

bool HIDJoystickController::isConnected() {
  return isReady();
}

HIDJoystickState * HIDJoystickController::getState() {
  return &state;
}

void HIDJoystickController::ParseHIDData(HID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf) {
  uint8_t axes[HIDJoystickState::NUM_AXES];
  uint8_t buttons[HIDJoystickState::NUM_BUTTONS];
  uint8_t hats[HIDJoystickState::NUM_HATS];

  memset(axes, 0, HIDJoystickState::NUM_AXES);
  memset(buttons, 0, HIDJoystickState::NUM_BUTTONS);
  memset(hats, 0, HIDJoystickState::NUM_HATS);

  int axisIdx = 0;
  int buttonIdx = 0;
  int hatIdx = 0;

  int bufIdx = 0;
  uint8_t bitIdx = 0;

  for (int idx = 0; idx < hidParser.getUsageDescriptionCount(); idx++) {
    HIDReportUsageDescription * desc = hidParser.getUsageDescription(idx);
    // axes
    if (desc->usage >= 0x30 && desc->usage <= 0x38) {
      int outputSize = HIDJoystickState::NUM_AXES - axisIdx;
      hidParser.parseHidDataBlock(desc, buf, &bufIdx, &bitIdx, &axes[axisIdx], &outputSize);
      axisIdx += outputSize;
    }
    // HAT
    else if (desc->usage == 0x39) {
      int outputSize = HIDJoystickState::NUM_HATS - hatIdx;
      hidParser.parseHidDataBlock(desc, buf, &bufIdx, &bitIdx, &hats[hatIdx], &outputSize);
      hatIdx += outputSize;
    }
    // buttons
    else if (desc->usagePage == 0x09) {
      int outputSize = HIDJoystickState::NUM_BUTTONS - buttonIdx;
      hidParser.parseHidDataBlock(desc, buf, &bufIdx, &bitIdx, &buttons[buttonIdx], &outputSize);
      buttonIdx += outputSize;
    }
    // parse but ignore other blocks
    else {
      int outputSize = 0;
      hidParser.parseHidDataBlock(desc, buf, &bufIdx, &bitIdx, NULL, &outputSize);
    }
  }

  // no changes
  if (!memcmp(state.axes, axes, HIDJoystickState::NUM_AXES) &&
  !memcmp(state.buttons, buttons, HIDJoystickState::NUM_BUTTONS) &&
  !memcmp(state.hats, hats, HIDJoystickState::NUM_HATS)) {
    return;
  }

  HID_JOYSTICK_DEBUG(print("RAW:"));
  for (uint16_t i = 0; i < len; i++) {
    HID_JOYSTICK_DEBUG(print(buf[i], HEX));
    if (((i+1) % 16) == 0) {
      HID_JOYSTICK_DEBUG(println());
    } else {
      HID_JOYSTICK_DEBUG(print(" "));
    }
  }
  HID_JOYSTICK_DEBUG(println());
  HID_JOYSTICK_DEBUG(print("AXES:"));
  for (int i = 0; i < axisIdx; i++) {
    HID_JOYSTICK_DEBUG(print(axes[i], HEX));
    HID_JOYSTICK_DEBUG(print(" "));
  }
  HID_JOYSTICK_DEBUG(println());
  HID_JOYSTICK_DEBUG(print("BUTTONS: "));
  for (int i = 0; i < buttonIdx; i++) {
    HID_JOYSTICK_DEBUG(print(buttons[i], HEX));
    HID_JOYSTICK_DEBUG(print(" "));
  }
  HID_JOYSTICK_DEBUG(println());
  HID_JOYSTICK_DEBUG(print("HATS: "));
  for (int i = 0; i < hatIdx; i++) {
    HID_JOYSTICK_DEBUG(print(hats[i], HEX));
    HID_JOYSTICK_DEBUG(print(" "));
  }
  HID_JOYSTICK_DEBUG(println());
  HID_JOYSTICK_DEBUG(println());

  memcpy(state.axes, axes, HIDJoystickState::NUM_AXES);
  memcpy(state.buttons, buttons, HIDJoystickState::NUM_BUTTONS);
  memcpy(state.hats, hats, HIDJoystickState::NUM_HATS);
  hidJoystickStateChange(&state);
};

uint32_t HIDJoystickController::Init(uint32_t parent, uint32_t port, uint32_t lowspeed) {
  hidParser.parseDeviceDescriptor(0, 0);
  return HIDUniversal::Init(parent, port, lowspeed);
}
