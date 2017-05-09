/* Copyright (C) 2014 Kristian Lauszus, TKJ Electronics. All rights reserved.

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

#include "SwitchParser.h"

enum DPADEnum {
        DPAD_UP = 0x0,
        DPAD_UP_RIGHT = 0x1,
        DPAD_RIGHT = 0x2,
        DPAD_RIGHT_DOWN = 0x3,
        DPAD_DOWN = 0x4,
        DPAD_DOWN_LEFT = 0x5,
        DPAD_LEFT = 0x6,
        DPAD_LEFT_UP = 0x7,
        DPAD_OFF = 0x8,
};

// To enable serial debugging see "settings.h"
#define PRINTREPORT // Uncomment to print the report send by the PS4 Controller

bool SwitchParser::checkDpad(ButtonEnum b) {
        switch (b) {
                case UP:
                        return switchData.btn.dpad == DPAD_LEFT_UP || switchData.btn.dpad == DPAD_UP || switchData.btn.dpad == DPAD_UP_RIGHT;
                case RIGHT:
                        return switchData.btn.dpad == DPAD_UP_RIGHT || switchData.btn.dpad == DPAD_RIGHT || switchData.btn.dpad == DPAD_RIGHT_DOWN;
                case DOWN:
                        return switchData.btn.dpad == DPAD_RIGHT_DOWN || switchData.btn.dpad == DPAD_DOWN || switchData.btn.dpad == DPAD_DOWN_LEFT;
                case LEFT:
                        return switchData.btn.dpad == DPAD_DOWN_LEFT || switchData.btn.dpad == DPAD_LEFT || switchData.btn.dpad == DPAD_LEFT_UP;
                default:
                        return false;
        }
}

bool SwitchParser::getButtonPress(ButtonEnum b) {
        if (b <= LEFT) // Dpad
                return checkDpad(b);
        else
                return switchData.btn.val & (1UL << pgm_read_byte(&SWITCH_BUTTONS[(uint8_t)b]));
}

bool SwitchParser::getButtonClick(ButtonEnum b) {
        uint32_t mask = 1UL << pgm_read_byte(&SWITCH_BUTTONS[(uint8_t)b]);        
        bool click = buttonClickState.val & mask;
        buttonClickState.val &= ~mask; // Clear "click" event
        return click;
}

uint8_t SwitchParser::getAnalogButton(ButtonEnum b) {
        if (b == L2) // These are the only analog buttons on the controller
                return switchData.trigger[0];
        else if (b == R2)
                return switchData.trigger[1];
        return 0;
}

uint8_t SwitchParser::getAnalogHat(AnalogHatEnum a) {
        return switchData.hatValue[(uint8_t)a];
}

void SwitchParser::Parse(uint8_t len, uint8_t *buf) {
        if (len > 1 && buf)  {
#ifdef PRINTREPORT
                Notify(PSTR("\r\n"), 0x80);
                for (uint8_t i = 0; i < len; i++) {
                        D_PrintHex<uint8_t > (buf[i], 0x80);
                        Notify(PSTR(" "), 0x80);
                }
#endif

                if (buf[0] == 0x01) // Check report ID
                        memcpy(&switchData, buf + 1, min((uint8_t)(len - 1), MFK_CASTUINT8T sizeof(switchData)));
                else if (buf[0] == 0x11) { // This report is send via Bluetooth, it has an offset of 2 compared to the USB data
                        if (len < 4) {
#ifdef DEBUG_USB_HOST
                                Notify(PSTR("\r\nReport is too short: "), 0x80);
                                D_PrintHex<uint8_t > (len, 0x80);
#endif
                                return;
                        }
                        memcpy(&switchData, buf + 3, min((uint8_t)(len - 3), MFK_CASTUINT8T sizeof(switchData)));
                } else {
#ifdef DEBUG_USB_HOST
                        Notify(PSTR("\r\nUnknown report id: "), 0x80);
                        D_PrintHex<uint8_t > (buf[0], 0x80);
#endif
                        return;
                }

                if (switchData.btn.val != oldButtonState.val) { // Check if anything has changed
                        buttonClickState.val = switchData.btn.val & ~oldButtonState.val; // Update click state variable
                        oldButtonState.val = switchData.btn.val;

                        // The DPAD buttons does not set the different bits, but set a value corresponding to the buttons pressed, we will simply set the bits ourself
                        uint8_t newDpad = 0;
                        if (checkDpad(UP))
                                newDpad |= 1 << UP;
                        if (checkDpad(RIGHT))
                                newDpad |= 1 << RIGHT;
                        if (checkDpad(DOWN))
                                newDpad |= 1 << DOWN;
                        if (checkDpad(LEFT))
                                newDpad |= 1 << LEFT;
                        if (newDpad != oldDpad) {
                                buttonClickState.dpad = newDpad & ~oldDpad; // Override values
                                oldDpad = newDpad;
                        }
                }
        }

        if (switchOutput.reportChanged)
                sendOutputReport(&switchOutput); // Send output report
}

void SwitchParser::Reset() {
        uint8_t i;
        for (i = 0; i < sizeof(switchData.hatValue); i++)
                switchData.hatValue[i] = 127; // Center value
        switchData.btn.val = 0;
        oldButtonState.val = 0;
        for (i = 0; i < sizeof(switchData.trigger); i++)
                switchData.trigger[i] = 0;
        for (i = 0; i < sizeof(switchData.xy)/sizeof(switchData.xy[0]); i++) {
                for (uint8_t j = 0; j < sizeof(switchData.xy[0].finger)/sizeof(switchData.xy[0].finger[0]); j++)
                        switchData.xy[i].finger[j].touching = 1; // The bit is cleared if the finger is touching the touchpad
        }

        switchData.btn.dpad = DPAD_OFF;
        oldButtonState.dpad = DPAD_OFF;
        buttonClickState.dpad = 0;
        oldDpad = 0;

        switchOutput.bigRumble = switchOutput.smallRumble = 0;
        switchOutput.r = switchOutput.g = switchOutput.b = 0;
        switchOutput.flashOn = switchOutput.flashOff = 0;
        switchOutput.reportChanged = false;
};

