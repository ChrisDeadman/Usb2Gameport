#ifndef _DEADZONE_H_
#define _DEADZONE_H_

#include <Arduino.h>

class Deadzone {

private:

  const float DEADZONE_STEP_VALUE = 0.1;
  const int DEADZONE_INC_MIN = 0;
  const int DEADZONE_INC_MAX = 5;

  int deadzoneInc = DEADZONE_STEP_VALUE;

public:

  float getValue() {
    return deadzoneInc * DEADZONE_STEP_VALUE;
  }

  void increase() {
    deadzoneInc = (deadzoneInc < DEADZONE_INC_MAX) ? (deadzoneInc + 1) : DEADZONE_INC_MIN;
  }

  void decrease() {
    deadzoneInc = (deadzoneInc > DEADZONE_INC_MIN) ? (deadzoneInc - 1) : DEADZONE_INC_MAX;
  }

  void apply(float * x, float * y) {
    float deadzone = getValue();
    if (deadzone < DEADZONE_STEP_VALUE) {
      return;
    }
    // calculate maginitude: c² = a² + b²
    // this approximation (0 ≤ a ≤ b) is faster and gives us max error of 1.04%:
    float xAbs = abs(*x);
    float yAbs = abs(*y);
    float a = min(xAbs, yAbs);
    float b = max(xAbs, yAbs);
    float magnitude = b + (0.428 * a * a / b);
    if (magnitude < deadzone) {
      *x = 0;
      *y = 0;
    } else {
      xAbs = ((xAbs - deadzone) / (1.0f - deadzone));
      yAbs = ((yAbs - deadzone) / (1.0f - deadzone));
      *x = (*x < 0) ? max(-xAbs, -1) : min(xAbs, 1);
      *y = (*y < 0) ? max(-yAbs, -1) : min(yAbs, 1);
    }
  }
};

#endif //_DEADZONE_H_
