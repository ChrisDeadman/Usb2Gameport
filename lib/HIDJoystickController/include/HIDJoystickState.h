#ifndef _HID_JOYSTICK_STATE_H_
#define _HID_JOYSTICK_STATE_H_

struct HIDJoystickState {
  static const uint8_t NUM_AXES = 8;
  static const uint8_t NUM_BUTTONS = 16;
  static const uint8_t NUM_HATS = 2;

  uint8_t axes[NUM_AXES];
  uint8_t buttons[NUM_BUTTONS];
  uint8_t hats[NUM_HATS];
};

#endif // _HID_JOYSTICK_STATE_H_
