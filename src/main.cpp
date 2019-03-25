#define JOY1_POT_CS_PIN 2
#define JOY2_POT_CS_PIN 3
#define JOY1_BUTTON1_PIN 4
#define JOY1_BUTTON2_PIN 5
#define JOY2_BUTTON1_PIN 6
#define JOY2_BUTTON2_PIN 7
#define STATUS_LED_PIN 9
//
// USB joystick controllers
//
#include <usbhub.h>
#include <hiduniversal.h>
#include "XboxController.h"
#include "GenericController.h"
USBHost usb;
USBHub usbHub(&usb); // so we can support controllers connected via hubs
XboxController xboxController(&usb);
GenericController genericController(&usb);

// Add all controllers here
JoyController * controllers[] = {
  &xboxController,
  &genericController
};
const int numControllers = sizeof(controllers)/sizeof(JoyController*);

//
// Digital Potentiometers
//
#include "MCP4251.h"
MCP4251 potJoy1(JOY1_POT_CS_PIN);
MCP4251 potJoy2(JOY2_POT_CS_PIN);
uint8_t minPotValue = 254;
uint8_t maxPotValue = 190;

// Helpers
#include "ButtonState.h"
#include "Deadzone.h"
#include "SoftPWM.h"
#include "Watchdog.h"
Deadzone deadzone;
SoftPWM softPwm;
Watchdog watchdog;
GameportState gameportState;
ButtonState conf1ButtonState;
ButtonState conf2ButtonState;
bool inConfMode = false;
bool inSetMaxPotValueMode = false;
bool inSetMaxDeadzoneMode = false;

uint8_t axisToPotValue(float axisValue) {
  const int8_t span = (maxPotValue - minPotValue) / 2;
  const uint8_t center = minPotValue + span;
  return center + (axisValue * span);
}

//
// Initial setup
//
void setup() {
  watchdog.setup();
  usb.Init();
  pinMode(STATUS_LED_PIN, OUTPUT);
  pinMode(JOY1_POT_CS_PIN, OUTPUT);
  pinMode(JOY2_POT_CS_PIN, OUTPUT);
  pinMode(JOY1_BUTTON1_PIN, OUTPUT);
  pinMode(JOY1_BUTTON2_PIN, OUTPUT);
  pinMode(JOY2_BUTTON1_PIN, OUTPUT);
  pinMode(JOY2_BUTTON2_PIN, OUTPUT);
  SPI.begin();

  // initialize gameport
  potJoy1.setWiper0(axisToPotValue(0));
  potJoy1.setWiper1(axisToPotValue(0));
  potJoy2.setWiper0(axisToPotValue(0));
  potJoy2.setWiper1(axisToPotValue(0));

  Serial1.begin(115200);
  Serial1.println("--------------------");
  Serial1.println("USB => Gameport V0.1");
  Serial1.println("--------------------");
}

//
// Configuration
//
bool handleConfiguration() {
  unsigned long tNow = millis();

  if (conf1ButtonState.isLongPressed() && conf2ButtonState.isLongPressed()) {
    inConfMode = !inConfMode;
    inSetMaxDeadzoneMode = false;
    inSetMaxPotValueMode = false;
    conf1ButtonState.resetLongPress();
    conf2ButtonState.resetLongPress();
  }
  if (inConfMode) {
    if (inSetMaxDeadzoneMode) {
      if (conf1ButtonState.isClicked()) {
        deadzone.increase(); // auto-rollover
      }
      digitalWrite(STATUS_LED_PIN, softPwm.pwm(deadzone.getValue() * 255));
      if (conf2ButtonState.isClicked()) {
        inSetMaxDeadzoneMode = false;
      }
    } else if (inSetMaxPotValueMode) {
      if (conf2ButtonState.isClicked()) {
        if (maxPotValue <= 0) maxPotValue = 254 - 16;
        else if (maxPotValue <= 16)  maxPotValue = 0;
        else maxPotValue -= 16;
      }
      digitalWrite(STATUS_LED_PIN, softPwm.pwm(maxPotValue));
      if (conf1ButtonState.isClicked()) {
        inSetMaxPotValueMode = false;
      }
    } else {
      digitalWrite(STATUS_LED_PIN, ((tNow % 1000) < 500) ? LOW : HIGH);
      if (conf1ButtonState.isLongPressed() && !conf2ButtonState.isPressed()) {
        inSetMaxDeadzoneMode = true;
        conf1ButtonState.resetLongPress();
      } else if (conf2ButtonState.isLongPressed() && !conf1ButtonState.isPressed()) {
        inSetMaxPotValueMode = true;
        conf2ButtonState.resetLongPress();
      }
    }
  }
  return inConfMode;
}

//
// Main loop
//
void loop() {
  // reset watchdog
  watchdog.reset();

  // poll USB devices
  usb.Task();

  // poll status of all connected controllers
  GameportState * joy1 = NULL;
  GameportState * joy2 = NULL;
  uint8_t numConnectedControllers = 0;
  for (int idx = 0; idx < numControllers; idx++) {
    JoyController * controller = controllers[idx];
    controller->task();
    if (controller->isConnected()) {
      if (numConnectedControllers < 1) {
        joy1 = controller->getState();
        conf1ButtonState.setPressed(controller->isConf1Pressed());
        conf2ButtonState.setPressed(controller->isConf2Pressed());
      } else {
        joy2 = controller->getState();
      }
      ++numConnectedControllers;
    }
  }

  // no controller connected, nothing to do
  if ((joy1 == NULL) && (joy2 == NULL)) {
    digitalWrite(STATUS_LED_PIN, softPwm.pwm(millis() % 255));
    return;
  }

  // update controller state
  gameportState.update(joy1, joy2);
  deadzone.apply(&gameportState.axis1, &gameportState.axis2);
  deadzone.apply(&gameportState.axis3, &gameportState.axis4);

  // update gameport
  potJoy1.setWiper0(axisToPotValue(gameportState.axis1));
  potJoy1.setWiper1(axisToPotValue(gameportState.axis2));
  potJoy2.setWiper0(axisToPotValue(gameportState.axis4));
  potJoy2.setWiper1(axisToPotValue(gameportState.axis3));
  digitalWrite(JOY1_BUTTON1_PIN, gameportState.button1 ? LOW : HIGH);
  digitalWrite(JOY1_BUTTON2_PIN, gameportState.button2 ? LOW : HIGH);
  digitalWrite(JOY2_BUTTON1_PIN, gameportState.button3 ? LOW : HIGH);
  digitalWrite(JOY2_BUTTON2_PIN, gameportState.button4 ? LOW : HIGH);

  // configuration
  if (handleConfiguration()) {
    return;
  }

  // LED indicates button press or axis values
  if (gameportState.button1 || gameportState.button2 || gameportState.button3 || gameportState.button4) {
    digitalWrite(STATUS_LED_PIN, HIGH);
  } else {
    uint8_t axis1Value = abs(gameportState.axis1) * 255;
    uint8_t axis2Value = abs(gameportState.axis2) * 255;
    uint8_t axis3Value = abs(gameportState.axis3) * 255;
    uint8_t axis4Value = abs(gameportState.axis4) * 255;
    uint8_t pwmValue = 0;
    if (axis1Value > pwmValue) pwmValue = axis1Value;
    if (axis2Value > pwmValue) pwmValue = axis2Value;
    if (axis3Value > pwmValue) pwmValue = axis3Value;
    if (axis4Value > pwmValue) pwmValue = axis4Value;
    digitalWrite(STATUS_LED_PIN, softPwm.pwm(pwmValue));
  }
}
