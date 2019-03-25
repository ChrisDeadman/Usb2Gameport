#ifndef _GENERIC_CONTROLLER_H_
#define _GENERIC_CONTROLLER_H_

#include "JoyController.h"
#include "HIDJoystickController.h"

class GenericController : public JoyController {

private:

  HIDJoystickController hidJoyController;
  GameportState controllerState;

public:

  GenericController(USBHost * usb) : hidJoyController(usb) {
  }

  bool isConnected() override {
    return hidJoyController.isConnected();
  }

  GameportState * getState() override {
    return &controllerState;
  }

  void task() override {
    if (!isConnected()) {
      return;
    }

    HIDJoystickState * hidJoyState = hidJoyController.getState();

    controllerState.axis1 = convertAxisToGamePortValue(hidJoyState->axes[0]);
    controllerState.axis2 = convertAxisToGamePortValue(hidJoyState->axes[1]);
    controllerState.axis3 = convertAxisToGamePortValue(hidJoyState->axes[2]);
    controllerState.axis4 = convertAxisToGamePortValue(hidJoyState->axes[3]);
    if ((hidJoyState->hats[0] >= 1) && (hidJoyState->hats[0] <=3)) controllerState.axis1 = 1; // LEFT
    if ((hidJoyState->hats[0] >= 5) && (hidJoyState->hats[0] <=7)) controllerState.axis1 = -1; // RIGHT
    if ((hidJoyState->hats[0] >= 3) && (hidJoyState->hats[0] <=5)) controllerState.axis2 = 1; // UP
    if ((hidJoyState->hats[0] == 7) || (hidJoyState->hats[0] <=1)) controllerState.axis2 = -1; // DOWN
    controllerState.button1 = hidJoyState->buttons[0] != 0;
    controllerState.button2 = hidJoyState->buttons[1] != 0;
    controllerState.button3 = hidJoyState->buttons[2] != 0;
    controllerState.button4 = hidJoyState->buttons[3] != 0;
    setConf1Pressed(hidJoyState->buttons[4] != 0);
    setConf2Pressed(hidJoyState->buttons[5] != 0);
  }

protected:

    inline float convertAxisToGamePortValue(uint8_t value) {
      return -1 + (min(value, 254) / 127.0f);
    }
};

#endif // _GENERIC_CONTROLLER_H_
