/*
 Example sketch for the PS4 Bluetooth library - developed by Kristian Lauszus
 For more information visit my blog: http://blog.tkjelectronics.dk/ or
 send me an e-mail:  kristianl@tkjelectronics.com
 */

#include "SwitchBT.h"
#include <usbhub.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#include <SPI.h>
#endif

USB Usb;
//USBHub Hub1(&Usb); // Some dongles have a hub inside
BTD Btd(&Usb); // You have to create the Bluetooth Dongle instance like so

/* You can create the instance of the PS4BT class in two ways */
// This will start an inquiry and then pair with the PS4 controller - you only have to do this once
// You will need to hold down the PS and Share button at the same time, the PS4 controller will then start to blink rapidly indicating that it is in pairing mode
//SwitchBT Switch(&Btd, PAIR);

// After that you can simply create the instance like so and then press the PS button on the device
SwitchBT Switch(&Btd);

bool printAngle, printTouch;
uint8_t oldL2Value, oldR2Value;

void setup() {
  Serial.begin(115200);
#if !defined(__MIPSEL__)
  while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
  if (Usb.Init() == -1) {
    Serial.print(F("\r\nOSC did not start"));
    while (1); // Halt
  }
  Serial.print(F("\r\nPS4 Bluetooth Library Started"));
}
void loop() {

  Usb.Task();

  if (Switch.connected()) {
    if (Switch.getAnalogHat(LeftHatX) > 137 || Switch.getAnalogHat(LeftHatX) < 117 || Switch.getAnalogHat(LeftHatY) > 137 || Switch.getAnalogHat(LeftHatY) < 117 || Switch.getAnalogHat(RightHatX) > 137 || Switch.getAnalogHat(RightHatX) < 117 || Switch.getAnalogHat(RightHatY) > 137 || Switch.getAnalogHat(RightHatY) < 117) {
      Serial.print(F("\r\nLeftHatX: "));
      Serial.print(Switch.getAnalogHat(LeftHatX));
      Serial.print(F("\tLeftHatY: "));
      Serial.print(Switch.getAnalogHat(LeftHatY));
      Serial.print(F("\tRightHatX: "));
      Serial.print(Switch.getAnalogHat(RightHatX));
      Serial.print(F("\tRightHatY: "));
      Serial.print(Switch.getAnalogHat(RightHatY));
    }

    if (Switch.getAnalogButton(L2) || Switch.getAnalogButton(R2)) { // These are the only analog buttons on the PS4 controller
      Serial.print(F("\r\nL2: "));
      Serial.print(Switch.getAnalogButton(L2));
      Serial.print(F("\tR2: "));
      Serial.print(Switch.getAnalogButton(R2));
    }
    if (Switch.getAnalogButton(L2) != oldL2Value || Switch.getAnalogButton(R2) != oldR2Value) // Only write value if it's different
      Switch.setRumbleOn(Switch.getAnalogButton(L2), Switch.getAnalogButton(R2));
    oldL2Value = Switch.getAnalogButton(L2);
    oldR2Value = Switch.getAnalogButton(R2);

    if (Switch.getButtonClick(PS)) {
      Serial.print(F("\r\nPS"));
      Switch.disconnect();
    }
    else {
      if (Switch.getButtonClick(TRIANGLE)) {
        Serial.print(F("\r\nTraingle"));
        Switch.setRumbleOn(RumbleLow);
      }
      if (Switch.getButtonClick(CIRCLE)) {
        Serial.print(F("\r\nCircle"));
        Switch.setRumbleOn(RumbleHigh);
      }
      if (Switch.getButtonClick(CROSS)) {
        Serial.print(F("\r\nCross"));
        Switch.setLedFlash(10, 10); // Set it to blink rapidly
      }
      if (Switch.getButtonClick(SQUARE)) {
        Serial.print(F("\r\nSquare"));
        Switch.setLedFlash(0, 0); // Turn off blinking
      }

      if (Switch.getButtonClick(UP)) {
        Serial.print(F("\r\nUp"));
        Switch.setLed(Red);
      } if (Switch.getButtonClick(RIGHT)) {
        Serial.print(F("\r\nRight"));
        Switch.setLed(Blue);
      } if (Switch.getButtonClick(DOWN)) {
        Serial.print(F("\r\nDown"));
        Switch.setLed(Yellow);
      } if (Switch.getButtonClick(LEFT)) {
        Serial.print(F("\r\nLeft"));
        Switch.setLed(Green);
      }

      if (Switch.getButtonClick(L1))
        Serial.print(F("\r\nL1"));
      if (Switch.getButtonClick(L3))
        Serial.print(F("\r\nL3"));
      if (Switch.getButtonClick(R1))
        Serial.print(F("\r\nR1"));
      if (Switch.getButtonClick(R3))
        Serial.print(F("\r\nR3"));

      if (Switch.getButtonClick(SHARE))
        Serial.print(F("\r\nShare"));
      if (Switch.getButtonClick(OPTIONS)) {
        Serial.print(F("\r\nOptions"));
        printAngle = !printAngle;
      }
      if (Switch.getButtonClick(TOUCHPAD)) {
        Serial.print(F("\r\nTouchpad"));
        printTouch = !printTouch;
      }

      if (printAngle) { // Print angle calculated using the accelerometer only
        Serial.print(F("\r\nPitch: "));
        Serial.print(Switch.getAngle(Pitch));
        Serial.print(F("\tRoll: "));
        Serial.print(Switch.getAngle(Roll));
      }

      if (printTouch) { // Print the x, y coordinates of the touchpad
        if (Switch.isTouching(0) || Switch.isTouching(1)) // Print newline and carriage return if any of the fingers are touching the touchpad
          Serial.print(F("\r\n"));
        for (uint8_t i = 0; i < 2; i++) { // The touchpad track two fingers
          if (Switch.isTouching(i)) { // Print the position of the finger if it is touching the touchpad
            Serial.print(F("X")); Serial.print(i + 1); Serial.print(F(": "));
            Serial.print(Switch.getX(i));
            Serial.print(F("\tY")); Serial.print(i + 1); Serial.print(F(": "));
            Serial.print(Switch.getY(i));
            Serial.print(F("\t"));
          }
        }
      }
    }
  }
}

