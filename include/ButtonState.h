#ifndef _BUTTON_STATE_H_
#define _BUTTON_STATE_H_

#define LONG_PRESS_TIME 1000

class ButtonState {

private:

  bool pressed = false;
  bool clicked = false;
  bool ignoreLongPress = false;

  unsigned long tPressedSince = 0;

public:

  void setPressed(bool pressed) {
    if (this->pressed && !ignoreLongPress && !isLongPressed() && !pressed) {
      this->clicked = true;
    } else {
      this->clicked = false;
    }

    if (!pressed) {
      ignoreLongPress = false;
    }

    if (!this->pressed && pressed) {
      this->tPressedSince = millis();
    }
    this->pressed = pressed;
  }

  bool isPressed() {
    return this->pressed;
  }

  bool isClicked() {
    return this->clicked;
  }

  bool isLongPressed() {
    return !ignoreLongPress && this->pressed && ((millis()-this->tPressedSince) > LONG_PRESS_TIME);
  }

  void resetLongPress() {
    if (isLongPressed()) {
      ignoreLongPress = true;
    }
  }
};

#endif //_BUTTON_STATE_H_
