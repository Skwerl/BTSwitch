/*////////////////////////////////////////////////////////////////////////////////////////////////*/
///////////////////////* Libraries *////////////////////////////////////////////////////////////////
/*////////////////////////////////////////////////////////////////////////////////////////////////*/

#include "SwitchBT.h"
#include <usbhub.h>

USB Usb;
BTD Btd(&Usb);

// To Pair:
//SwitchBT Switch(&Btd, PAIR);

// After Pair:
SwitchBT Switch(&Btd);

/*////////////////////////////////////////////////////////////////////////////////////////////////*/
///////////////////////* Switch Parsing *///////////////////////////////////////////////////////////
/*////////////////////////////////////////////////////////////////////////////////////////////////*/

byte* Report;

struct SwitchButtons {
  bool Idle;
  bool U_Button;
  bool D_Button;
  bool L_Button;
  bool R_Button;
  bool SL_Button;
  bool SR_Button;
  bool L_Trigger;
  bool ZL_Trigger;
  bool R_Trigger;
  bool ZR_Trigger;
  bool Stick_Button;
  bool Minus_Button;
  bool Plus_Button;
  bool Capture_Button;
  int Analog_Stick;
};
SwitchButtons reset = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
SwitchButtons state = reset;

void handleEvent() {

  if (Report[0] == 0 && Report[1] == 0 && Report[2] == 8) {

    allButtonsReleased();
    state.Idle = 1;

  } else {

    Serial.println("Event Detected!");

    state.Idle = 0;

    switch(Report[0]) {
      case 4:
        state.U_Button = 1;
        break;
      case 2:
        state.D_Button = 1;
        break;
      case 1:
        state.L_Button = 1;
        break;
      case 8:
        state.R_Button = 1;
        break;
      case 16:
        state.SL_Button = 1;
        break;
      case 32:
        state.SR_Button = 1;
        break;
    }
    switch(Report[1]) {
      case 64:
        state.L_Trigger = 1;
        break;
      case 128:
        state.ZL_Trigger = 1;
        break;
      case 1:
        state.Minus_Button = 1;
        break;
      case 32:
        state.Capture_Button = 1;
        break;
      case 4:
        state.Stick_Button = 1;
        break;
    }
    switch(Report[2]) {
      case 8:
        state.Analog_Stick = 0;
        break;
      case 7:
        state.Analog_Stick = 45;
        break;
      case 0:
        state.Analog_Stick = 90;
        break;
      case 1:
        state.Analog_Stick = 135;
        break;
      case 2:
        state.Analog_Stick = 180;
        break;
      case 3:
        state.Analog_Stick = 225;
        break;
      case 4:
        state.Analog_Stick = 270;
        break;
      case 5:
        state.Analog_Stick = 315;
        break;
      case 6:
        state.Analog_Stick = 360;
        break;
    }

  }

}

void allButtonsReleased() {
  state = reset;
}

/*////////////////////////////////////////////////////////////////////////////////////////////////*/
///////////////////////* Arduino *//////////////////////////////////////////////////////////////////
/*////////////////////////////////////////////////////////////////////////////////////////////////*/

void setup() {
  Serial.begin(115200);
  if (Usb.Init() == -1) {
    Serial.print(F("\r\nOSC did not start"));
    while (1); // Halt
  }
  Serial.print(F("\r\nSwitch Bluetooth Library Started"));
}

void loop() {

  Usb.Task();

  if (Switch.connected()) {

    Report = Switch.Report;
    handleEvent();
    
  }
}

