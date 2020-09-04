/*********************************************************************
 XIAO FootKey

 This project uses Yamaha foot switch FC5 as a USB keyboard.
 For more information, please check here.
 https://github.com/carcon999/footkey
 Copyright (c) 2020 @carcon999
 Released under the MIT license

 We used the Adafruit library.
 Thanks to Adafruit.
 MIT license, check LICENSE for more information
 Copyright (c) 2019 Ha Thach for Adafruit Industries
 All text above, and the splash screen below must be included in
 any redistribution

*********************************************************************/
#include "Adafruit_TinyUSB.h"

/* 
 *  This sketch demonstrates USB HID keyboard.
 * - PIN A0-A3 is used to send digit '0' to '3' respectively
 * - We have confirmed the operation using Yamaha foot switch FC5. 
 *   Please note that the logic may be inverted if other switches are used.
 */

// HID report descriptor using TinyUSB's template
// Single Report (no ID) descriptor
uint8_t const desc_hid_report[] =
{
  TUD_HID_REPORT_DESC_KEYBOARD(),
};

Adafruit_USBD_HID usb_hid;

#define INPUT_COUNT  (4)

// Array of pins and its keycode
// For keycode definition see BLEHidGeneric.h
const uint8_t pins[INPUT_COUNT]    = { A0, A1, A2, A3};

// If you want to change the corresponding key code, you need to change this value.
const uint8_t hidcode[INPUT_COUNT] = { HID_KEY_0, HID_KEY_1, HID_KEY_2, HID_KEY_3};
static uint8_t pin_enable[INPUT_COUNT];

// the setup function runs once when you press reset or power the board
void setup()
{
  usb_hid.setPollInterval(2);
  usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
  usb_hid.begin();

  // Set up pin as input
  for (uint8_t i=0; i<INPUT_COUNT; i++)
  {
    pinMode(pins[i], INPUT_PULLUP);
  }

  // wait until device mounted
  while( !USBDevice.mounted() ) delay(1);

  // It looks LOW when the plug is inserted.
  // The foot switch used is the Yamaha foot switch FC5.
  for (uint8_t i=0; i<INPUT_COUNT; i++)
  {
    pin_enable[i] = (digitalRead(pins[i]) == LOW);
  }  
}


void loop()
{
  // poll gpio once each 2 ms
  delay(2);

  if ( !usb_hid.ready() ) return;

  static bool keyPressedPreviously = false;
  bool anyKeyPressed = false;

  uint8_t count=0;
  uint8_t keycode[6] = { 0 };

  // scan normal key and send report
  for(uint8_t i=0; i < INPUT_COUNT; i++)
  {
    if(!pin_enable[i]){
      continue;
    }

    if ( 1 == digitalRead(pins[i]) )
    {
      // if pin is active (high), add its hid code to key report
      keycode[count++] = hidcode[i];

      // 6 is max keycode per report
      if (count == 6)
      {
        usb_hid.keyboardReport(0, 0, keycode);
        delay(2); // delay for report to send out

        // reset report
        count = 0;
        memset(keycode, 0, 6);
      }

      // used later
      anyKeyPressed = true;
      keyPressedPreviously = true;
    }
  }

  // Send any remaining keys (not accumulated up to 6)
  if ( count )
  {
    usb_hid.keyboardReport(0, 0, keycode);
  }

  // Send All-zero report to indicate there is no keys pressed
  // Most of the time, it is, though we don't need to send zero report
  // every loop(), only a key is pressed in previous loop()
  if ( !anyKeyPressed && keyPressedPreviously )
  {
    keyPressedPreviously = false;
    usb_hid.keyboardRelease(0);
  }
}
