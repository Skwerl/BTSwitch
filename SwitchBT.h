#ifndef _switchbt_h_
#define _switchbt_h_

#include "SwHID.h"

/**
   This class implements support for the PS4 controller via Bluetooth.
   It uses the BTHID class for all the Bluetooth communication.
*/
class SwitchBT : public SwHID {
  
  public:
    
    SwitchBT(BTD *p, bool pair = false, const char *pin = "0000") :
      SwHID(p, pair, pin) {
    };
    
    bool connected() {
      return SwHID::connected;
    };

    byte Report[3]; 

  protected:

    virtual void ParseSwHIDData(uint8_t len, uint8_t *buf) {

      Report[0] = buf[0];
      Report[1] = buf[1];
      Report[2] = buf[2];
      
    };

    virtual void OnInitSwHID() {
      return;
    };

    virtual void ResetSwHID() {
      return;
    };

  private:
    
    void HID_Command(uint8_t *data, uint8_t nbytes) {
      pBtd->L2CAP_Command(hci_handle, data, nbytes, control_scid[0], control_scid[1]);
    };

};
#endif
