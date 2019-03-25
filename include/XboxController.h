#ifndef _XBOX_CONTROLLER_H_
#define _XBOX_CONTROLLER_H_

#include <XBOXRECV.h>
#include "JoyController.h"

class XboxController : public JoyController {

private:

  XBOXRECV xboxRecv;
  GameportState controllerState;
  bool conf1Pressed;
  bool conf2Pressed;

public:

  XboxController(USBHost * usb) : xboxRecv(usb) { }

  bool isConnected() override {
    return xboxRecv.XboxReceiverConnected;
  }

  GameportState * getState() override {
    return &controllerState;
  }

  void task() override {
    if (!isConnected()) {
      return;
    }

    GameportState joy1;
    GameportState joy2;
    uint8_t numControllers = 0;

    conf1Pressed = false;
    conf2Pressed = false;

    for (uint8_t controller = 0; controller < 4; controller++) {
      if (xboxRecv.Xbox360Connected[controller]) {
        if (numControllers < 1) {
          getControllerState(controller, &joy1);
          setConf1Pressed(xboxRecv.getButtonPress(R1, controller));
          setConf2Pressed(xboxRecv.getButtonPress(L1, controller));
        } else {
          getControllerState(controller, &joy2);
        }
        ++numControllers;
      }
    }

    controllerState.update(&joy1, (numControllers > 1) ? &joy2 : NULL);
  }

private:

  void getControllerState(uint8_t controller, GameportState * controllerState) {
    controllerState->axis1 = convertAxisToGamePortValue(xboxRecv.getAnalogHat(LeftHatX, controller));
    controllerState->axis2 = -convertAxisToGamePortValue(xboxRecv.getAnalogHat(LeftHatY, controller)); // y is inverted
    controllerState->axis3 = convertAxisToGamePortValue(xboxRecv.getAnalogHat(RightHatX, controller));
    controllerState->axis4 = -convertAxisToGamePortValue(xboxRecv.getAnalogHat(RightHatY, controller)); // y is inverted
    if (xboxRecv.getButtonPress(LEFT, controller)) controllerState->axis1 = -1;
    if (xboxRecv.getButtonPress(RIGHT, controller)) controllerState->axis1 = 1;
    if (xboxRecv.getButtonPress(UP, controller)) controllerState->axis2 = -1;
    if (xboxRecv.getButtonPress(DOWN, controller)) controllerState->axis2 = 1;
    controllerState->button1 = xboxRecv.getButtonPress(A, controller);
    controllerState->button2 = xboxRecv.getButtonPress(B, controller);
    controllerState->button3 = xboxRecv.getButtonPress(X, controller);
    controllerState->button4 = xboxRecv.getButtonPress(Y, controller);
  }

protected:

    inline float convertAxisToGamePortValue(int16_t value) {
      if (value <= INT16_MIN) {
        value = INT16_MIN + 1;
      }
      bool invert = value < 0;
      float result = (float)abs(value) / INT16_MAX;
      return invert ? -result : result;
    }
};

#endif // _XBOX_CONTROLLER_H_
