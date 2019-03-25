#ifndef _JOY_CONTROLLER_H_
#define _JOY_CONTROLLER_H_

#include "GameportState.h"

class JoyController {

private:

  bool conf1Pressed = false;
  bool conf2Pressed = false;

protected:

  void setConf1Pressed(bool pressed) {
    conf1Pressed = pressed;
  }

  void setConf2Pressed(bool pressed) {
    conf2Pressed = pressed;
  }

public:

  bool isConf1Pressed() {
    return conf1Pressed;
  }

  bool isConf2Pressed() {
    return conf2Pressed;
  }

  virtual bool isConnected() = 0;
  virtual GameportState * getState() = 0;
  virtual void task() = 0;
};

#endif // _JOY_CONTROLLER_H_
