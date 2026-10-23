# padawanJoycon: padawan360 with Nintendo Switch Joy-Cons

This is Dan Kraus's [padawan360](https://github.com/dankraus/padawan360), unchanged in behaviour, with one addition: the body sketch for the **Arduino Mega** can be driven with a pair of **Nintendo Switch Joy-Cons** instead of an Xbox 360 controller.

**Why.** The Xbox 360 wireless receiver has not been made for years, and the cheap "compatible" receivers often do not work with padawan360. Joy-Cons are still sold everywhere, charge over USB-C, and talk plain Bluetooth. Held together, the two halves have every button of an Xbox 360 controller, so nothing in padawan360's button map has to move.

> **Status: bench-tested only.** All 17 buttons and both sticks are checked on the bench with two Joy-Cons. Nothing has been driven on a droid yet. Do the first runs with the wheels off the ground.

## What you need

- **Arduino Mega 2560** (or Mega ADK) and the same USB Host Shield. The Uno cannot fit the Bluetooth stack: it alone takes 88% of the Uno's flash and 76% of its RAM.
- A **USB Bluetooth dongle** in place of the Xbox receiver. The same ones used by Shadow and PS3 Nav droids work, for instance CSR8510 based dongles.
- Two **Joy-Cons**, a left and a right.
- **USB Host Shield Library 2.0, version 1.7.0 or newer** (from the Library Manager). The older copy in `libraries/` does not have the Joy-Con support. 1.7.0 also works with the Xbox 360 receiver, so you only need one library.

## Choosing the controller

One line in `padawan360_body/padawan360_body_mega_i2c_ino/padawan360_body_mega_i2c/ControllerConfig.h`:

```c
//#define USE_XBOX360               // original behaviour: Xbox 360 controller + wireless receiver
#define USE_JOYCON                  // two Joy-Cons + a Bluetooth dongle
```

With `USE_XBOX360` the sketch compiles to exactly the same size as the original padawan360: the Xbox code path is untouched.

## Button map

The face buttons go **by position, not by letter**, so your hands do what they did on the Xbox controller. The bottom button does what A did, even though it says B.

| Xbox 360 | Joy-Cons | | Xbox 360 | Joy-Cons |
|---|---|---|---|---|
| left stick | left stick | | right stick | right stick |
| d-pad | left d-pad | | A (bottom) | B (bottom) |
| LB | L | | B (right) | A (right) |
| LT | ZL | | X (left) | Y (left) |
| L3 | left stick click | | Y (top) | X (top) |
| BACK | Minus | | RB | R |
| START | Plus | | RT | ZR |
| XBOX | Home | | R3 | right stick click |

Capture is not used by padawan360, so it is the pairing button. The speed LEDs work too: one, two or three player LEDs on both Joy-Cons for low, medium and high speed, and all four flashing while the drive is disabled, as the Xbox ring rotates.

## Pairing

No re-uploading.

- **The first time**, the droid opens a pairing session by itself. Hold the small sync button on the rail of the **left** Joy-Con until it connects and vibrates, then the **right** one.
- **Every day**, press any button on each Joy-Con and they call the droid back. After a disconnect, wait about ten seconds first.
- **To pair again**, hold **Capture** (left) or **Home** (right) for 5 seconds, or reset the Arduino three times in a row.
- **A Joy-Con used on a Switch** is caught when it pairs: you are asked to hold sync again, and the second time it is accepted.

Each step plays an MP3 Trigger track, set in `ControllerConfig.h`. While a session is open the droid does not move.

The droid only counts the controller as connected when **both** halves are connected and talking, so if either one drops, or goes quiet for more than 250 ms, the motors stop, as they do when the Xbox controller drops.

## What changed in the sketch

**Your own padawan360 sketch?** [MIGRATING_TO_JOYCON.md](MIGRATING_TO_JOYCON.md) walks you through adding the Joy-Cons to it, step by step.

Four small blocks in `padawan360_body_mega_i2c.ino`, all behind `#ifdef`: the includes, the `Xbox` object, starting the Joy-Cons and pairing at the end of `setup()`, and three lines at the top of `loop()`. Every one of the sketch's `Xbox.getButtonClick()`, `Xbox.getAnalogHat()` and other calls is unchanged, because `JoyConXbox.h` answers the same calls the Xbox receiver class does.

| File | What it is |
|---|---|
| `ControllerConfig.h` | Choose the controller, pairing sounds, options. The only file you edit. |
| `JoyConXbox.h` | Two Joy-Cons that answer like the Xbox 360 receiver. |
| `JoyConController.h` | One Joy-Con: full report mode, stick centring and drift correction, which half it is. |
| `JoyConPairing.h` | Pairing without re-uploading, and reconnecting by button. |
| `DroidController.h` | The controller interface `JoyConController` is built on. |

These Joy-Con files are shared with [ShadowMD Lite](https://github.com/Fernando-Beltran/ShadowMDLite) and the [Shadow MD fork](https://github.com/Fernando-Beltran/SHADOW_MD_EEBEL), where the Joy-Con support was first written and bench-tested.

## Bench test first

`JoyConBench/` runs the same controller layer and prints every button by its Xbox name, and the four stick values on the Xbox scale, to the serial monitor at 115200. It drives nothing. Use it to check your Joy-Cons before the droid.

**Check your sticks there.** Joy-Con sticks are known for drift. A small offset is corrected automatically while the stick rests, but a stick that rests far from centre needs replacing: one of our test Joy-Cons rested 35 to 45% off centre.

**Safety: the stick only drives once it has been seen centred.** A worn Joy-Con can rest far off centre (one of ours rested 40% off), which would drive the droid the moment it pairs. So after every connection the stick reads zero until it sits in the centre, and the serial console says why: `JOY-L: stick rests far from centre (held, or drift). Stick off until it centres.` Move the stick and let go; when it centres you get `stick centred, driving again`. A stick that freezes off centre for 2 seconds is switched off the same way. Buttons keep working. What no software can do is tell a stick that rests at 25% and keeps wobbling from a thumb holding it there: **if your stick does not come back to zero in the bench test, replace it** (a spare stick costs a few euros) before driving with it.

## Credit

padawan360 by Dan Kraus, https://github.com/dankraus/padawan360. The Joy-Con protocol details come from dekuNukem's reverse engineering notes, https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering. Thank you both.
