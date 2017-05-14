/* Copyright (C) 2013 Kristian Lauszus, TKJ Electronics. All rights reserved.

  This software may be distributed and modified under the terms of the GNU
  General Public License version 2 (GPL2) as published by the Free Software
  Foundation and appearing in the file GPL2.TXT included in the packaging of
  this file. Please note that GPL2 Section 2[b] requires that all works based
  on this software must also be made publicly available under the terms of
  the GPL2 ("Copyleft").

  Contact information
  -------------------

  Kristian Lauszus, TKJ Electronics
  Web      :  http://www.tkjelectronics.com
  e-mail   :  kristianl@tkjelectronics.com
*/

#include "SwHID.h"

SwHID::SwHID(BTD *p, bool pair, const char *pin) : BluetoothService(p), protocolMode(USB_HID_BOOT_PROTOCOL) {

  pBtd->pairWithHIDDevice = pair;
  pBtd->btdPin = pin;

  control_dcid[0] = 0x70; // 0x0070
  control_dcid[1] = 0x00;
  interrupt_dcid[0] = 0x71; // 0x0071
  interrupt_dcid[1] = 0x00;

  Reset();

}

void SwHID::Reset() {
  connected = false;
  activeConnection = false;
  l2cap_event_flag = 0;
  l2cap_state = L2CAP_WAIT;
  ResetSwHID();
}

void SwHID::disconnect() {
  pBtd->l2cap_disconnection_request(hci_handle, ++identifier, interrupt_scid, interrupt_dcid);
  Reset();
  l2cap_state = L2CAP_INTERRUPT_DISCONNECT;
}

void SwHID::ACLData(uint8_t* l2capinbuf) {
  
  if (!pBtd->l2capConnectionClaimed && pBtd->incomingHIDDevice && !connected && !activeConnection) {
    if (l2capinbuf[8] == L2CAP_CMD_CONNECTION_REQUEST) {
      if ((l2capinbuf[12] | (l2capinbuf[13] << 8)) == HID_CTRL_PSM) {
        pBtd->incomingHIDDevice = false;
        pBtd->l2capConnectionClaimed = true; // Claim that the incoming connection belongs to this service
        activeConnection = true;
        hci_handle = pBtd->hci_handle; // Store the HCI Handle for the connection
        l2cap_state = L2CAP_WAIT;
      }
    }
  }

  if (checkHciHandle(l2capinbuf, hci_handle)) {
    if ((l2capinbuf[6] | (l2capinbuf[7] << 8)) == 0x0001U) {
      if (l2capinbuf[8] == L2CAP_CMD_COMMAND_REJECT) {
        #ifdef DEBUG_USB_HOST
          Notify(PSTR("\r\nL2CAP Command Rejected - Reason: "), 0x80);
          D_PrintHex<uint8_t > (l2capinbuf[13], 0x80);
          Notify(PSTR(" "), 0x80);
          D_PrintHex<uint8_t > (l2capinbuf[12], 0x80);
          Notify(PSTR(" "), 0x80);
          D_PrintHex<uint8_t > (l2capinbuf[17], 0x80);
          Notify(PSTR(" "), 0x80);
          D_PrintHex<uint8_t > (l2capinbuf[16], 0x80);
          Notify(PSTR(" "), 0x80);
          D_PrintHex<uint8_t > (l2capinbuf[15], 0x80);
          Notify(PSTR(" "), 0x80);
          D_PrintHex<uint8_t > (l2capinbuf[14], 0x80);
        #endif
      } else if (l2capinbuf[8] == L2CAP_CMD_CONNECTION_RESPONSE) {
        if (((l2capinbuf[16] | (l2capinbuf[17] << 8)) == 0x0000) && ((l2capinbuf[18] | (l2capinbuf[19] << 8)) == SUCCESSFUL)) {
          if (l2capinbuf[14] == control_dcid[0] && l2capinbuf[15] == control_dcid[1]) {
            identifier = l2capinbuf[9];
            control_scid[0] = l2capinbuf[12];
            control_scid[1] = l2capinbuf[13];
            l2cap_set_flag(L2CAP_FLAG_CONTROL_CONNECTED);
          } else if (l2capinbuf[14] == interrupt_dcid[0] && l2capinbuf[15] == interrupt_dcid[1]) {
            identifier = l2capinbuf[9];
            interrupt_scid[0] = l2capinbuf[12];
            interrupt_scid[1] = l2capinbuf[13];
            l2cap_set_flag(L2CAP_FLAG_INTERRUPT_CONNECTED);
          }
        }
      } else if (l2capinbuf[8] == L2CAP_CMD_CONNECTION_REQUEST) {
          if ((l2capinbuf[12] | (l2capinbuf[13] << 8)) == HID_CTRL_PSM) {
            identifier = l2capinbuf[9];
            control_scid[0] = l2capinbuf[14];
            control_scid[1] = l2capinbuf[15];
            l2cap_set_flag(L2CAP_FLAG_CONNECTION_CONTROL_REQUEST);
          } else if ((l2capinbuf[12] | (l2capinbuf[13] << 8)) == HID_INTR_PSM) {
            identifier = l2capinbuf[9];
            interrupt_scid[0] = l2capinbuf[14];
            interrupt_scid[1] = l2capinbuf[15];
            l2cap_set_flag(L2CAP_FLAG_CONNECTION_INTERRUPT_REQUEST);
          }
      } else if (l2capinbuf[8] == L2CAP_CMD_CONFIG_RESPONSE) {
        if ((l2capinbuf[16] | (l2capinbuf[17] << 8)) == 0x0000) {
          if (l2capinbuf[12] == control_dcid[0] && l2capinbuf[13] == control_dcid[1]) {
            identifier = l2capinbuf[9];
            l2cap_set_flag(L2CAP_FLAG_CONFIG_CONTROL_SUCCESS);
          } else if (l2capinbuf[12] == interrupt_dcid[0] && l2capinbuf[13] == interrupt_dcid[1]) {
            identifier = l2capinbuf[9];
            l2cap_set_flag(L2CAP_FLAG_CONFIG_INTERRUPT_SUCCESS);
          }
        }
      } else if (l2capinbuf[8] == L2CAP_CMD_CONFIG_REQUEST) {
        if (l2capinbuf[12] == control_dcid[0] && l2capinbuf[13] == control_dcid[1]) {
          pBtd->l2cap_config_response(hci_handle, l2capinbuf[9], control_scid);
        } else if (l2capinbuf[12] == interrupt_dcid[0] && l2capinbuf[13] == interrupt_dcid[1]) {
          pBtd->l2cap_config_response(hci_handle, l2capinbuf[9], interrupt_scid);
        }
      } else if (l2capinbuf[8] == L2CAP_CMD_DISCONNECT_REQUEST) {
        if (l2capinbuf[12] == control_dcid[0] && l2capinbuf[13] == control_dcid[1]) {
          #ifdef DEBUG_USB_HOST
            Notify(PSTR("\r\nDisconnect Request: Control Channel"), 0x80);
          #endif
          identifier = l2capinbuf[9];
          pBtd->l2cap_disconnection_response(hci_handle, identifier, control_dcid, control_scid);
          Reset();
        } else if (l2capinbuf[12] == interrupt_dcid[0] && l2capinbuf[13] == interrupt_dcid[1]) {
          #ifdef DEBUG_USB_HOST
            Notify(PSTR("\r\nDisconnect Request: Interrupt Channel"), 0x80);
          #endif
          identifier = l2capinbuf[9];
          pBtd->l2cap_disconnection_response(hci_handle, identifier, interrupt_dcid, interrupt_scid);
          Reset();
        }
      } else if (l2capinbuf[8] == L2CAP_CMD_DISCONNECT_RESPONSE) {
        if (l2capinbuf[12] == control_scid[0] && l2capinbuf[13] == control_scid[1]) {
          identifier = l2capinbuf[9];
          l2cap_set_flag(L2CAP_FLAG_DISCONNECT_CONTROL_RESPONSE);
        } else if (l2capinbuf[12] == interrupt_scid[0] && l2capinbuf[13] == interrupt_scid[1]) {
          identifier = l2capinbuf[9];
          l2cap_set_flag(L2CAP_FLAG_DISCONNECT_INTERRUPT_RESPONSE);
        }
      } else {
        identifier = l2capinbuf[9];
        Notify(PSTR("\r\nL2CAP Unknown Signaling Command: "), 0x80);
        D_PrintHex<uint8_t > (l2capinbuf[8], 0x80);
      }
    } else if (l2capinbuf[6] == interrupt_dcid[0] && l2capinbuf[7] == interrupt_dcid[1]) {

      // The data we need from the Joy-Con is these three bytes:
      byte report[] = {l2capinbuf[10],l2capinbuf[11],l2capinbuf[12]};
      ParseSwHIDData(sizeof(report), report);

    } else if (l2capinbuf[6] == control_dcid[0] && l2capinbuf[7] == control_dcid[1]) {
      //Notify(PSTR("\r\nL2CAP Control: "), 0x80);
      for (uint16_t i = 0; i < ((uint16_t)l2capinbuf[5] << 8 | l2capinbuf[4]); i++) {
        //D_PrintHex<uint8_t > (l2capinbuf[i + 8], 0x80);
        //Notify(PSTR(" "), 0x80);
      }
    } else {
      Notify(PSTR("\r\nUnsupported L2CAP Data; Channel ID: "), 0x80);
      D_PrintHex<uint8_t > (l2capinbuf[7], 0x80);
      Notify(PSTR(" "), 0x80);
      D_PrintHex<uint8_t > (l2capinbuf[6], 0x80);
      Notify(PSTR("\r\nData: "), 0x80);
      Notify(PSTR("\r\n"), 0x80);
      for (uint16_t i = 0; i < ((uint16_t)l2capinbuf[5] << 8 | l2capinbuf[4]); i++) {
        D_PrintHex<uint8_t > (l2capinbuf[i + 8], 0x80);
        Notify(PSTR(" "), 0x80);
      }
    }
    L2CAP_task();
  }
}

void SwHID::L2CAP_task() {
  switch (l2cap_state) {
    case L2CAP_CONTROL_SUCCESS:
      if (l2cap_check_flag(L2CAP_FLAG_CONFIG_CONTROL_SUCCESS)) {
        #ifdef DEBUG_USB_HOST
          Notify(PSTR("\r\nHID Control Successfully Configured"), 0x80);
        #endif
        setProtocol();
        l2cap_state = L2CAP_INTERRUPT_SETUP;
      }
      break;
    case L2CAP_INTERRUPT_SETUP:
      if (l2cap_check_flag(L2CAP_FLAG_CONNECTION_INTERRUPT_REQUEST)) {
        #ifdef DEBUG_USB_HOST
          //Notify(PSTR("\r\nHID Interrupt Incoming Connection Request"), 0x80);
        #endif
        pBtd->l2cap_connection_response(hci_handle, identifier, interrupt_dcid, interrupt_scid, PENDING);
        delay(1);
        pBtd->l2cap_connection_response(hci_handle, identifier, interrupt_dcid, interrupt_scid, SUCCESSFUL);
        identifier++;
        delay(1);
        pBtd->l2cap_config_request(hci_handle, identifier, interrupt_scid);
        l2cap_state = L2CAP_INTERRUPT_CONFIG_REQUEST;
      }
      break;

    case L2CAP_CONTROL_CONNECT_REQUEST:
      if (l2cap_check_flag(L2CAP_FLAG_CONTROL_CONNECTED)) {
        #ifdef DEBUG_USB_HOST
          Notify(PSTR("\r\nSend HID Control Config Request"), 0x80);
        #endif
        identifier++;
        pBtd->l2cap_config_request(hci_handle, identifier, control_scid);
        l2cap_state = L2CAP_CONTROL_CONFIG_REQUEST;
      }
      break;

    case L2CAP_CONTROL_CONFIG_REQUEST:
      if (l2cap_check_flag(L2CAP_FLAG_CONFIG_CONTROL_SUCCESS)) {
        setProtocol();
        delay(1);
        #ifdef DEBUG_USB_HOST
          Notify(PSTR("\r\nSend HID Interrupt Connection Request"), 0x80);
        #endif
        identifier++;
        pBtd->l2cap_connection_request(hci_handle, identifier, interrupt_dcid, HID_INTR_PSM);
        l2cap_state = L2CAP_INTERRUPT_CONNECT_REQUEST;
      }
      break;

    case L2CAP_INTERRUPT_CONNECT_REQUEST:
      if (l2cap_check_flag(L2CAP_FLAG_INTERRUPT_CONNECTED)) {
        #ifdef DEBUG_USB_HOST
          Notify(PSTR("\r\nSend HID Interrupt Config Request"), 0x80);
        #endif
        identifier++;
        pBtd->l2cap_config_request(hci_handle, identifier, interrupt_scid);
        l2cap_state = L2CAP_INTERRUPT_CONFIG_REQUEST;
      }
      break;

    case L2CAP_INTERRUPT_CONFIG_REQUEST:
      if (l2cap_check_flag(L2CAP_FLAG_CONFIG_INTERRUPT_SUCCESS)) {
        #ifdef DEBUG_USB_HOST
          Notify(PSTR("\r\nHID Channels Established"), 0x80);
        #endif
        pBtd->connectToHIDDevice = false;
        pBtd->pairWithHIDDevice = false;
        connected = true;
        onInit();
        l2cap_state = L2CAP_DONE;
      }
      break;

    case L2CAP_DONE:
      break;

    case L2CAP_INTERRUPT_DISCONNECT:
      if (l2cap_check_flag(L2CAP_FLAG_DISCONNECT_INTERRUPT_RESPONSE)) {
        #ifdef DEBUG_USB_HOST
          Notify(PSTR("\r\nDisconnected Interrupt Channel"), 0x80);
        #endif
        identifier++;
        pBtd->l2cap_disconnection_request(hci_handle, identifier, control_scid, control_dcid);
        l2cap_state = L2CAP_CONTROL_DISCONNECT;
      }
      break;

    case L2CAP_CONTROL_DISCONNECT:
      if (l2cap_check_flag(L2CAP_FLAG_DISCONNECT_CONTROL_RESPONSE)) {
        #ifdef DEBUG_USB_HOST
          Notify(PSTR("\r\nDisconnected Control Channel"), 0x80);
        #endif
        pBtd->hci_disconnect(hci_handle);
        hci_handle = -1; // Reset handle
        l2cap_event_flag = 0; // Reset flags
        l2cap_state = L2CAP_WAIT;
      }
      break;
  }
}

void SwHID::Run() {
  switch (l2cap_state) {
    case L2CAP_WAIT:
      if (pBtd->connectToHIDDevice && !pBtd->l2capConnectionClaimed && !connected && !activeConnection) {
        pBtd->l2capConnectionClaimed = true;
        activeConnection = true;
        #ifdef DEBUG_USB_HOST
          Notify(PSTR("\r\nSend HID Control Connection Request"), 0x80);
        #endif
        hci_handle = pBtd->hci_handle;
        l2cap_event_flag = 0;
        identifier = 0;
        pBtd->l2cap_connection_request(hci_handle, identifier, control_dcid, HID_CTRL_PSM);
        l2cap_state = L2CAP_CONTROL_CONNECT_REQUEST;
      } else if (l2cap_check_flag(L2CAP_FLAG_CONNECTION_CONTROL_REQUEST)) {
        #ifdef DEBUG_USB_HOST
          //Notify(PSTR("\r\nHID Control Incoming Connection Request"), 0x80);
        #endif
        pBtd->l2cap_connection_response(hci_handle, identifier, control_dcid, control_scid, PENDING);
        delay(1);
        pBtd->l2cap_connection_response(hci_handle, identifier, control_dcid, control_scid, SUCCESSFUL);
        identifier++;
        delay(1);
        pBtd->l2cap_config_request(hci_handle, identifier, control_scid);
        l2cap_state = L2CAP_CONTROL_SUCCESS;
      }
      break;
  }
}

/************************************************************/
/*                    HID Commands                          */
/************************************************************/

void SwHID::setProtocol() {
  #ifdef DEBUG_USB_HOST
    //Notify(PSTR("\r\nSet protocol mode: "), 0x80);
    //D_PrintHex<uint8_t > (protocolMode, 0x80);
  #endif
  if (protocolMode != USB_HID_BOOT_PROTOCOL && protocolMode != HID_RPT_PROTOCOL) {
    #ifdef DEBUG_USB_HOST
      Notify(PSTR("\r\nNot a valid protocol mode. Using Boot protocol instead."), 0x80);
    #endif
    protocolMode = USB_HID_BOOT_PROTOCOL; // Use Boot Protocol by default
  }
  uint8_t command = 0x70 | protocolMode; // Set Protocol, see Bluetooth HID specs page 33
  pBtd->L2CAP_Command(hci_handle, &command, 1, control_scid[0], control_scid[1]);
}
