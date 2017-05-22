/*////////////////////////////////////////////////////////////////////////////////////////////////*/
///////////////////////* Libraries *////////////////////////////////////////////////////////////////
/*////////////////////////////////////////////////////////////////////////////////////////////////*/

#include "SwitchBT.h"
#include <usbhub.h>

USB Usb;
BTD Btd(&Usb);

// To Pair:
SwitchBT Switch(&Btd, PAIR);

// After Pair:
//SwitchBT Switch(&Btd);

bool bluetoothInit = false;

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
  bool Select_Button;
  bool Action_Button;
  int Analog_Stick;
};
SwitchButtons reset = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
SwitchButtons state = reset;

void handleEvent() {
  
  if (Report[0] == 0 && Report[1] == 0 && Report[2] == 8) {

    allButtonsReleased();
    state.Idle = 1;

  } else {

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
        state.R_Trigger = 1;
        break;
      case 128:
        state.ZL_Trigger = 1;
        state.ZR_Trigger = 1;
        break;
      case 1:
      case 2:
        state.Select_Button = 1;
        break;
      case 32:
        state.Action_Button = 1;
        break;
      case 4:
      case 8:
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

  if (!bluetoothInit) {
    if (state.Analog_Stick > 0) {
      return;    
    } else {
      bluetoothInit = true;
      Serial.println("");
      Serial.println("Bluetooth Connection Established");
    }
  } else {
    
    Serial.print(state.Idle);
    Serial.print(state.U_Button);
    Serial.print(state.D_Button);
    Serial.print(state.L_Button);
    Serial.print(state.R_Button);
    Serial.print(state.SL_Button);
    Serial.print(state.SR_Button);
    Serial.print(state.L_Trigger);
    Serial.print(state.ZL_Trigger);
    Serial.print(state.Stick_Button);
    Serial.print(state.Select_Button);
    Serial.print(state.Action_Button);
    Serial.print(state.Analog_Stick);
    Serial.println("");
    
    //doAction();
    
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

