#include "SoftPWM.h"

bool SoftPWM::pwm(uint8_t value) {
  unsigned long tNow = micros();
  unsigned long tDelta = tNow - tStart;

  if (tDelta >= tPeriod) {
    tStart = tNow;
    tDelta = 0;
  }

  return (tDelta < (value * tFactor));
}
