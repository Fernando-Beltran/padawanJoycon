// =======================================================================================
// JoyConBench: check the controller on the bench, without the droid.
//
// Runs the same controller layer as padawan360_body_mega_i2c, and prints every button
// press, by its Xbox 360 name, and the four stick values on the Xbox scale (-32768..32767)
// to the serial monitor at 115200. Drives nothing: no Sabertooth, no SyRen, no MP3 Trigger.
//
// Pick the controller in ControllerConfig.h, as in the main sketch. With an Xbox 360
// controller you should see the same names and numbers as with the Joy-Cons.
//
// Serial commands: 'l' steps through the LEDs the sketch uses (LED1, LED2, LED3, ROTATING),
// 'p' starts pairing (Joy-Cons only).
// =======================================================================================

#include "ControllerConfig.h"
#ifdef USE_XBOX360
  #include <XBOXRECV.h>
#else
  #include "JoyConXbox.h"
  #include "JoyConPairing.h"
#endif

#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif

USB Usb;
#ifdef USE_XBOX360
XBOXRECV Xbox(&Usb);
#else
BTD Btd(&Usb);
JoyConController joyLeft(&Btd, JOYCON_LEFT, "LEFT", JOYCON_PAIRING_MODE);
JoyConController joyRight(&Btd, JOYCON_RIGHT, "RIGHT", JOYCON_PAIRING_MODE);
JoyConXbox Xbox(&joyLeft, &joyRight);
JoyConPairing joyPairing(&Btd, &joyLeft, &joyRight);
#endif

struct NamedButton { ButtonEnum b; const char* name; };
const NamedButton BUTTONS[] = {
  { UP, "UP" }, { DOWN, "DOWN" }, { LEFT, "LEFT" }, { RIGHT, "RIGHT" },
  { A, "A" }, { B, "B" }, { X, "X" }, { Y, "Y" },
  { L1, "LB (L1)" }, { L2, "LT (L2)" }, { L3, "L3" },
  { R1, "RB (R1)" }, { R2, "RT (R2)" }, { R3, "R3" },
  { BACK, "BACK" }, { START, "START" }, { XBOX, "XBOX" },
};

bool wasConnected = false;
uint32_t lastPrint = 0;
uint8_t ledStep = 0;

void setup()
{
  Serial.begin(115200);
  while (!Serial);
  Serial.println(F("\r\n=== padawanJoycon controller bench ==="));
#ifdef USE_XBOX360
  Serial.println(F("Controller: Xbox 360 + wireless receiver"));
#else
  Serial.println(F("Controller: two Joy-Cons + Bluetooth dongle"));
#endif
  if (Usb.Init() == -1)
  {
    Serial.println(F("USB host did not start. HALT."));
    while (1);
  }
#ifdef USE_JOYCON
  joyLeft.setStickRange(JOYCON_STICK_RANGE);
  joyRight.setStickRange(JOYCON_STICK_RANGE);
  Xbox.begin();
  joyPairing.begin([](JoyConPairEvent e)
  {
    switch (e)
    {
      case JCP_STARTED:         Serial.println(F("PAIRING started: hold sync on a Joy-Con")); break;
      case JCP_PAIRED_ONE:      Serial.println(F("PAIRING paired one, now the other")); break;
      case JCP_DONE:            Serial.println(F("PAIRING done")); break;
      case JCP_TIMEOUT:         Serial.println(F("PAIRING timed out")); break;
      case JCP_HOLD_SYNC_AGAIN: Serial.println(F("PAIRING that Joy-Con remembers another host: hold sync again")); break;
    }
  }, JOYCON_PAIRING_MODE);
#endif
  Serial.println(F("Waiting for the controller..."));
}

void loop()
{
  Usb.Task();
#ifdef USE_JOYCON
  Xbox.update();
  joyPairing.update();
  Xbox.hold(joyPairing.active());
#endif

  if (Serial.available())
  {
    const char c = Serial.read();
#ifdef USE_JOYCON
    if (c == 'p') joyPairing.request();
#endif
    if (c == 'l')
    {
      switch (ledStep++ % 4)
      {
        case 0: Xbox.setLedOn(LED1, 0); Serial.println(F("LED1 (low speed)"));  break;
        case 1: Xbox.setLedOn(LED2, 0); Serial.println(F("LED2 (mid speed)"));  break;
        case 2: Xbox.setLedOn(LED3, 0); Serial.println(F("LED3 (high speed)")); break;
        case 3: Xbox.setLedMode(ROTATING, 0); Serial.println(F("ROTATING (drive off)")); break;
      }
    }
  }

  const bool connected = Xbox.XboxReceiverConnected && Xbox.Xbox360Connected[0];
  if (connected != wasConnected)
  {
    wasConnected = connected;
    Serial.println(connected ? F("CONNECTED") : F("NOT CONNECTED: motors would stop"));
    if (connected) Xbox.setLedMode(ROTATING, 0);   // what the sketch does on connect
  }
  if (!connected) return;

  for (uint8_t i = 0; i < sizeof(BUTTONS) / sizeof(BUTTONS[0]); i++)
    if (Xbox.getButtonClick(BUTTONS[i].b, 0))
    {
      Serial.print(F("click "));
      Serial.println(BUTTONS[i].name);
    }

  if (millis() - lastPrint > 500)
  {
    lastPrint = millis();
    Serial.print(F("LX ")); Serial.print(Xbox.getAnalogHat(LeftHatX, 0));
    Serial.print(F("  LY ")); Serial.print(Xbox.getAnalogHat(LeftHatY, 0));
    Serial.print(F("  RX ")); Serial.print(Xbox.getAnalogHat(RightHatX, 0));
    Serial.print(F("  RY ")); Serial.print(Xbox.getAnalogHat(RightHatY, 0));
    Serial.print(F("  held:"));
    for (uint8_t i = 0; i < sizeof(BUTTONS) / sizeof(BUTTONS[0]); i++)
      if (Xbox.getButtonPress(BUTTONS[i].b, 0)) { Serial.print(' '); Serial.print(BUTTONS[i].name); }
    Serial.println();
  }
}
