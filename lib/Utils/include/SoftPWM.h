#ifndef _SIMPLE_SOFT_PWM_H_
#define _SIMPLE_SOFT_PWM_H_

#include <Arduino.h>

class SoftPWM
{

private:

  const unsigned long tPeriod = 19890;
  const unsigned long tFactor = tPeriod / 255;

  unsigned long tStart;

public:

  bool pwm(uint8_t value);
};

#endif
