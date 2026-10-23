// ---------------------------------------------------------------------------------------
// JoyConXbox.h
//
// Two Nintendo Switch Joy-Cons that answer like the Xbox 360 wireless receiver.
//
// padawan360 talks to the controller through eight calls of the XBOXRECV class:
// getButtonPress(), getButtonClick(), getAnalogHat(), setLedOn(), setLedMode(),
// disconnect(), and the XboxReceiverConnected and Xbox360Connected[] members. This class
// answers those same calls from a left and a right Joy-Con, so the sketch keeps its whole
// button map and every line of its drive code. Held together the two halves have the
// buttons of an Xbox 360 controller, one for one:
//
//   Xbox 360          Joy-Cons                     Xbox 360          Joy-Cons
//   left stick        left stick                   right stick       right stick
//   d-pad             left d-pad                   A  (bottom)       B  (bottom)
//   LB  (L1)          L                            B  (right)        A  (right)
//   LT  (L2)          ZL                           X  (left)         Y  (left)
//   L3                left stick click             Y  (top)          X  (top)
//   BACK              Minus                        RB  (R1)          R
//   START             Plus                         RT  (R2)          ZR
//   XBOX (guide)      Home                         R3                right stick click
//
// The face buttons go by position, not by letter: the bottom button does what A did,
// whatever is printed on it. Capture is left free: holding it starts pairing.
//
// The controller counts as connected only when both halves are connected and both have
// reported within JOYCON_LINK_TIMEOUT_MS, so the sketch stops the motors, as it does when
// the Xbox controller drops, if either half goes quiet.
// ---------------------------------------------------------------------------------------

#ifndef JOYCON_XBOX_H
#define JOYCON_XBOX_H

#include <xboxEnums.h>          // ROTATING and the other LED modes the sketch uses
#include "JoyConController.h"

#ifndef JOYCON_LINK_TIMEOUT_MS
  #define JOYCON_LINK_TIMEOUT_MS  250   // a Joy-Con in full mode reports every ~15 ms
#endif

class JoyConXbox
{
public:
  JoyConXbox(JoyConController* a, JoyConController* b) : _jc{ a, b } {}

  // Same members as XBOXRECV. The "receiver" is the Bluetooth dongle, there once USB is up.
  bool XboxReceiverConnected = false;
  bool Xbox360Connected[4] = { false, false, false, false };

  void begin()
  {
    _jc[0]->begin();
    _jc[1]->begin();
  }

  // Call every loop, right after Usb.Task().
  void update()
  {
    _jc[0]->update();
    _jc[1]->update();
    XboxReceiverConnected = true;

    JoyConController* l = half(SLOT_FOOT);
    JoyConController* r = half(SLOT_DOME);
    const bool ok = l && r && live(l) && live(r) && !_held;
    if (ok && !Xbox360Connected[0]) _resendLed = true;
    Xbox360Connected[0] = ok;

    uint32_t now = 0;
    if (ok)
      for (uint8_t i = 0; i < N_BUTTONS; i++)
      {
        const ButtonEnum b = (ButtonEnum)pgm_read_byte(&PHYSICAL[i]);
        if (l->raw()->getButtonPress(b) || r->raw()->getButtonPress(b)) now |= 1UL << i;
      }
    _click |= now & ~_now;     // like XBOXRECV: set when a button goes down
    _now = now;
    if (!ok) _click = 0;

    // A half that has just connected shows its own player LED; put the sketch's back.
    if (_resendLed && ok && !_jc[0]->isCalibrating() && !_jc[1]->isCalibrating())
    {
      _resendLed = false;
      applyLed();
    }
  }

  // While pairing, the controller counts as disconnected, so the sketch stops the motors.
  void hold(bool on)
  {
    _held = on;
    if (on) { Xbox360Connected[0] = false; _now = 0; _click = 0; }
  }

  bool getButtonPress(ButtonEnum b, uint8_t controller = 0)
  {
    const int8_t i = index(b);
    return controller == 0 && i >= 0 && (_now & (1UL << i));
  }

  // Like XBOXRECV, reading a click clears it.
  bool getButtonClick(ButtonEnum b, uint8_t controller = 0)
  {
    const int8_t i = index(b);
    if (controller != 0 || i < 0) return false;
    const bool c = _click & (1UL << i);
    _click &= ~(1UL << i);
    return c;
  }

  // Xbox 360 scale: -32768..32767, up and right positive.
  int16_t getAnalogHat(AnalogHatEnum a, uint8_t controller = 0)
  {
    if (controller != 0 || !Xbox360Connected[0]) return 0;
    JoyConController* jc = half((a == LeftHatX || a == LeftHatY) ? SLOT_FOOT : SLOT_DOME);
    if (!jc) return 0;
    const int32_t v = (a == LeftHatX || a == RightHatX) ? (int32_t)jc->x() : -(int32_t)jc->y();
    const int32_t scaled = v * (32768L / DC_STICK_MAX);
    return (int16_t)constrain(scaled, -32768L, 32767L);
  }

  // The sketch shows the drive speed with LED1, LED2 or LED3: one, two or three player LEDs
  // on both halves. ROTATING (drive disabled) flashes all four.
  void setLedOn(LEDEnum led, uint8_t controller = 0)
  {
    if (controller != 0) return;
    switch (led)
    {
      case LED1: _led = 0x01; break;
      case LED2: _led = 0x03; break;
      case LED3: _led = 0x07; break;
      case LED4: _led = 0x0F; break;
      default:   _led = 0x00; break;
    }
    applyLed();
  }

  void setLedMode(LEDModeEnum, uint8_t controller = 0)
  {
    if (controller != 0) return;
    _led = 0xF0;
    applyLed();
  }

  void disconnect(uint8_t controller = 0)
  {
    if (controller != 0) return;
    _jc[0]->disconnect();
    _jc[1]->disconnect();
  }

private:
  // The Xbox buttons the sketch can ask for, and the Joy-Con button in the same place.
  static const uint8_t N_BUTTONS = 17;
  static int8_t index(ButtonEnum b)
  {
    switch (b)
    {
      case UP:    return 0;   case RIGHT: return 1;   case DOWN:  return 2;   case LEFT: return 3;
      case L1:    return 4;   case L2:    return 5;   case L3:    return 6;
      case R1:    return 7;   case R2:    return 8;   case R3:    return 9;
      case BACK:  return 10;  case START: return 11;  case XBOX:  return 12;
      case A:     return 13;  case B:     return 14;  case X:     return 15;  case Y:    return 16;
      default:    return -1;
    }
  }
  static const uint8_t PHYSICAL[N_BUTTONS] PROGMEM;

  JoyConController* half(DroidSlot slot)
  {
    for (uint8_t i = 0; i < 2; i++)
      if (_jc[i]->isConnected() && _jc[i]->preferredSlot() == slot) return _jc[i];
    return NULL;
  }

  static bool live(JoyConController* jc) { return jc->millisSinceLastMessage() < JOYCON_LINK_TIMEOUT_MS; }

  void applyLed()
  {
    _jc[0]->setPlayerLeds(_led);
    _jc[1]->setPlayerLeds(_led);
  }

  JoyConController* _jc[2];
  uint32_t _now = 0;
  uint32_t _click = 0;
  uint8_t  _led = 0;
  bool     _held = false;
  bool     _resendLed = false;
};

const uint8_t JoyConXbox::PHYSICAL[JoyConXbox::N_BUTTONS] PROGMEM =
{
  UP, RIGHT, DOWN, LEFT,
  L, ZL, L3,              // LB, LT, L3
  R, ZR, R3,              // RB, RT, R3
  MINUS, PLUS, HOME,      // BACK, START, XBOX
  B, A, Y, X              // Xbox A, B, X, Y by position: bottom, right, left, top
};

#endif // JOYCON_XBOX_H
