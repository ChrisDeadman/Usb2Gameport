#ifndef _GAMEPORT_STATE_H_
#define _GAMEPORT_STATE_H_

struct GameportState {
  float axis1 = 0;
  float axis2 = 0;
  float axis3 = 0;
  float axis4 = 0;
  bool button1 = false;
  bool button2 = false;
  bool button3 = false;
  bool button4 = false;

  void update(GameportState * joy1, GameportState * joy2) {
    if (joy1 != NULL) {
      axis1 = joy1->axis1;
      axis2 = joy1->axis2;
      axis3 = joy1->axis3;
      axis4 = joy1->axis4;
      button1 = joy1->button1;
      button2 = joy1->button2;
      button3 = joy1->button3;
      button4 = joy1->button4;
    }
    if (joy2 != NULL) {
      axis3 = joy2->axis1;
      axis4 = joy2->axis2;
      button3 = joy2->button1;
      button4 = joy2->button2;
    }
  }
};

#endif // _GAMEPORT_STATE_H_
