// ---------------------------------------------------------------------------------------
// ControllerConfig.h
//
// THIS IS THE ONLY FILE YOU NEED TO EDIT TO CHANGE CONTROLLER.
//
// padawan360 was written for the Xbox 360 controller and its wireless USB receiver. This
// fork lets you run the same sketch, with the same button map, using a pair of Nintendo
// Switch Joy-Cons instead, through a Bluetooth dongle plugged into the same USB Host Shield.
//
// Pick one of the two lines below and leave the other commented out.
// ---------------------------------------------------------------------------------------

#ifndef CONTROLLER_CONFIG_H
#define CONTROLLER_CONFIG_H

// --- Choose your controller -------------------------------------------------------------

//#define USE_XBOX360               // original behaviour: Xbox 360 controller + wireless receiver
#define USE_JOYCON                  // two Joy-Cons + a Bluetooth dongle

// --- Joy-Con options, ignored unless USE_JOYCON is selected -----------------------------

// Pairing needs no change here. On the first boot the droid pairs by itself: hold the
// small sync button on the left Joy-Con, then on the right one. To pair again later, hold
// Capture (left) or Home (right) for 5 seconds, or reset the Arduino three times in a row.
// true forces pairing on every boot.
#define JOYCON_PAIRING_MODE  false

// Sounds played during pairing, as MP3 Trigger track numbers. Change them to suit your SD
// card.
#define JOYCON_SOUND_STARTED  52   // pairing on: hold sync on a Joy-Con
#define JOYCON_SOUND_PAIRED   53   // one paired, now the other one
#define JOYCON_SOUND_DONE     21   // both paired
#define JOYCON_SOUND_TIMEOUT  53   // gave up after 60 seconds
#define JOYCON_SOUND_AGAIN    52   // that Joy-Con still remembers a Switch: hold sync again

// Raw stick deflection treated as full scale. Lower it if the droid reaches full speed
// before the stick is all the way over, raise it if it never gets there.
#define JOYCON_STICK_RANGE   1000

// Capture and Home are the buttons the sketch does not use for anything else, so holding
// one of them starts pairing. Minus and Plus, the default elsewhere, are BACK and START here.
#define JOYCON_PAIR_COMBO    (DB_B)

// ---------------------------------------------------------------------------------------
// Joy-Con button map. The face buttons go by position, so your hands do what they did on
// the Xbox controller, whatever letter is printed on the button.
//
//   Xbox 360      Joy-Cons                  Xbox 360      Joy-Cons
//   left stick    left stick                right stick   right stick
//   d-pad         left d-pad                A (bottom)    B (bottom)
//   LB            L                         B (right)     A (right)
//   LT            ZL                        X (left)      Y (left)
//   L3            left stick click          Y (top)       X (top)
//   BACK          Minus                     RB            R
//   START         Plus                      RT            ZR
//   XBOX          Home                      R3            right stick click
// ---------------------------------------------------------------------------------------

#if defined(USE_XBOX360) && defined(USE_JOYCON)
  #error "Pick ONE controller in ControllerConfig.h, not both."
#endif
#if !defined(USE_XBOX360) && !defined(USE_JOYCON)
  #error "Pick a controller in ControllerConfig.h."
#endif

#endif // CONTROLLER_CONFIG_H
