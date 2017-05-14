#ifndef _swhid_h_
#define _swhid_h_

#include "BTD.h"
#include "hidboot.h"

class SwHID : public BluetoothService {
  public:

    SwHID(BTD *p, bool pair = false, const char *pin = "0000");

    void disconnect();

    void setProtocolMode(uint8_t mode) {
      protocolMode = mode;
    };

    bool connected;

    void pair(void) {
      if (pBtd)
        pBtd->pairWithHID();
    };

  protected:

    void ACLData(uint8_t* ACLData);

    void Run();

    void Reset();

    void onInit() {
      if (pFuncOnInit)
        pFuncOnInit(); // Call the user function
      OnInitSwHID();
    };

    virtual void ParseSwHIDData(uint8_t len, uint8_t *buf) {
      return;
    };
    
    virtual void OnInitSwHID() {
      return;
    };
    virtual void ResetSwHID() {
      return;
    }

    /** L2CAP source CID for HID_Control */
    uint8_t control_scid[2];

    /** L2CAP source CID for HID_Interrupt */
    uint8_t interrupt_scid[2];

  private:

    /** Set report protocol. */
    void setProtocol();
    uint8_t protocolMode;

    void L2CAP_task(); // L2CAP state machine

    bool activeConnection; // Used to indicate if it already has established a connection

    /* Variables used for L2CAP communication */
    uint8_t control_dcid[2]; // L2CAP device CID for HID_Control - Always 0x0070
    uint8_t interrupt_dcid[2]; // L2CAP device CID for HID_Interrupt - Always 0x0071
    uint8_t l2cap_state;
};
#endif
