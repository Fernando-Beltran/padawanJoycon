# Moving your own padawan360 to Joy-Cons

This guide is for you if your droid already runs padawan360's **Mega** body sketch (Dan Kraus's, or your own version of it) and you want to drive it with two Nintendo Switch Joy-Cons instead of the Xbox 360 controller. **You keep your sketch.** Your button map, your sounds, your speeds and your I2C commands stay as they are: you add five files and paste four small blocks.

**If you never changed the body sketch**, there is a shorter way: use `padawan360_body_mega_i2c` from this repo, copy your own settings into it (speeds, Sabertooth and SyRen settings, sound counts), and go to step 7.

**Mega only.** The Bluetooth stack does not fit an Uno.

> **Before you start.** Bench-tested only so far: nothing has been driven on a droid yet. Make a copy of your working sketch folder, keep it, and do the first runs with the wheels off the ground.

---

## 1. Back up

Copy your whole sketch folder somewhere safe. If anything goes wrong, that copy is your droid as it was this morning.

## 2. Hardware: swap the receiver for a Bluetooth dongle

Unplug the Xbox 360 wireless receiver from the USB Host Shield and plug in a **USB Bluetooth dongle**. The same ones used by Shadow and PS3 Nav droids work, for instance CSR8510 based dongles. Nothing else changes.

## 3. Install the right library

Joy-Cons need **USB Host Shield Library 2.0, version 1.7.0 or newer**. The copy in padawan360's `libraries` folder is older and has no Joy-Con support.

1. Close the Arduino IDE.
2. In `Documents/Arduino/libraries/`, delete (or move out of `libraries`) the `USB_Host_Shield_20` / `USB_Host_Shield_2.0` folder you installed from padawan360.
3. Open the IDE, go to *Library Manager*, search for **USB Host Shield Library 2.0** and install the latest version.

1.7.0 also works with the Xbox 360 receiver, so you can switch back with one line (step 4) without touching the library again.

## 4. Add the five files

Download these files from [`padawan360_body_mega_i2c`](padawan360_body/padawan360_body_mega_i2c_ino/padawan360_body_mega_i2c) in this repo and put them in your sketch folder, next to your `.ino`:

| File | What it does |
|---|---|
| `ControllerConfig.h` | Choose the controller, pairing sounds, options. The only one you edit. |
| `JoyConXbox.h` | Two Joy-Cons that answer like the Xbox 360 receiver. |
| `JoyConController.h` | One Joy-Con. |
| `JoyConPairing.h` | Pairing without re-uploading. |
| `DroidController.h` | The controller interface the Joy-Con is built on. |

Open `ControllerConfig.h` and select the Joy-Cons:

```c
//#define USE_XBOX360
#define USE_JOYCON
```

In the same file, `JOYCON_SOUND_STARTED`, `_PAIRED`, `_DONE`, `_TIMEOUT` and `_AGAIN` are the MP3 Trigger tracks played while pairing. Point them at sounds you have on your SD card.

## 5. Four blocks in the sketch

**a. The include.** Find:

```c
#include <XBOXRECV.h>
```

and replace it with:

```c
// Controller: Xbox 360 or Joy-Cons. Pick it in ControllerConfig.h.
#include "ControllerConfig.h"
#ifdef USE_XBOX360
  #include <XBOXRECV.h>
#else
  #include "JoyConXbox.h"
  #include "JoyConPairing.h"
#endif
```

**b. The controller object.** Find:

```c
XBOXRECV Xbox(&Usb);
```

and replace it with:

```c
#ifdef USE_XBOX360
XBOXRECV Xbox(&Usb);
#else
// Two Joy-Cons that answer like the Xbox receiver: the rest of the sketch keeps calling
// Xbox.getButtonClick(), Xbox.getAnalogHat() and the rest as it always has.
BTD Btd(&Usb);
JoyConController joyLeft(&Btd, JOYCON_LEFT, "LEFT", JOYCON_PAIRING_MODE);
JoyConController joyRight(&Btd, JOYCON_RIGHT, "RIGHT", JOYCON_PAIRING_MODE);
JoyConXbox Xbox(&joyLeft, &joyRight);
JoyConPairing joyPairing(&Btd, &joyLeft, &joyRight);
#endif
```

**c. The end of `setup()`.** Paste this as the last thing in `setup()`, after `Usb.Init()`:

```c
#ifdef USE_JOYCON
  joyLeft.setStickRange(JOYCON_STICK_RANGE);
  joyRight.setStickRange(JOYCON_STICK_RANGE);
  Xbox.begin();
  // Pairing: first boot, Capture or Home held for 5 s, or three resets in a row. Each
  // step plays a sound. A lambda, so the IDE does not hoist a prototype above the type.
  joyPairing.begin([](JoyConPairEvent e)
  {
    switch (e)
    {
      case JCP_STARTED:         mp3Trigger.play(JOYCON_SOUND_STARTED); break;
      case JCP_PAIRED_ONE:      mp3Trigger.play(JOYCON_SOUND_PAIRED);  break;
      case JCP_DONE:            mp3Trigger.play(JOYCON_SOUND_DONE);    break;
      case JCP_TIMEOUT:         mp3Trigger.play(JOYCON_SOUND_TIMEOUT); break;
      case JCP_HOLD_SYNC_AGAIN: mp3Trigger.play(JOYCON_SOUND_AGAIN);   break;
    }
  }, JOYCON_PAIRING_MODE);
#endif
```

If your sketch plays sounds another way, or calls the MP3 Trigger object something other than `mp3Trigger`, change those five lines to match.

**d. The top of `loop()`.** Right after `Usb.Task();`, paste:

```c
#ifdef USE_JOYCON
  Xbox.update();
  joyPairing.update();
  Xbox.hold(joyPairing.active());   // while pairing, the droid does not move
#endif
```

That is all. Everything else in your sketch (every `Xbox.getButtonClick()`, `Xbox.getButtonPress()`, `Xbox.getAnalogHat()`, `Xbox.setLedOn()`, `Xbox.setLedMode()`, `Xbox.disconnect()`, and the `Xbox.XboxReceiverConnected` / `Xbox.Xbox360Connected[0]` checks) keeps working, because `JoyConXbox` answers the same calls.

**If it does not compile** and the error names an `Xbox.` call that is not in that list (for instance rumble or battery level, if you added them), that call has no Joy-Con version yet. Put it inside `#ifdef USE_XBOX360` ... `#endif`.

## 6. The button map

The face buttons go **by position, not by letter**, so your hands do what they did on the Xbox controller: the bottom button does what A did, even though it says B. The full map is in [JOYCON.md](JOYCON.md#button-map). Capture is the pairing button.

## 7. Compile and upload

Board: **Arduino Mega or Mega 2560**. Compile, upload, and keep the serial monitor closed while you pair (it is not needed).

## 8. Pair, on the bench first

- **The first time**, the droid opens a pairing session by itself and plays `JOYCON_SOUND_STARTED`. Hold the small sync button on the rail of the **left** Joy-Con until it connects and vibrates, then the **right** one.
- **Every day**, press any button on each Joy-Con and they call the droid back. After a disconnect, wait about ten seconds first.
- **To pair again**, hold **Capture** (left) or **Home** (right) for 5 seconds, or reset the Arduino three times in a row.
- **A Joy-Con used on a Switch** is caught when it pairs: you are asked to hold sync again, and the second time it is accepted.

Want to check the Joy-Cons before the droid? [`JoyConBench/`](JoyConBench) prints every button by its Xbox name and the sticks on the Xbox scale, and drives nothing.

## 9. Check before you drive

With the wheels off the ground:

1. **Sticks.** Forward on the left stick drives forward. Check the turn direction too. A stick that rests far from centre (Joy-Con drift) needs replacing.
2. **Dead man.** Turn one Joy-Con off with its sync button: the motors must stop, as they do when the Xbox controller drops.
3. **Pairing.** While a pairing session is open, the droid must not move.
4. **Sounds and dome.** Run through your usual buttons.

## Going back to the Xbox 360

In `ControllerConfig.h`, comment `USE_JOYCON` and uncomment `USE_XBOX360`, plug the receiver back in, and upload. With `USE_XBOX360` the sketch compiles to the same size as before the change.

## Credit

padawan360 by Dan Kraus, https://github.com/dankraus/padawan360. The Joy-Con protocol details come from dekuNukem's reverse engineering notes, https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering. Thank you both.
