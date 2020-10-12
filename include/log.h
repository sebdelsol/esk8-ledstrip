#pragma once

#include <Streaming.h>
#include <FastledCfg.h>
#include <FastLED.h>

extern Stream& _log;

// -- Stream extension
// -- _HEXS() for signed hexadecimal print : _HEXS(-16) gives -10 instead of FFFFFFF0 if an int
// -- _HEXS() works for CRGB too

template<typename T>
struct _SignedHEX
{
  T val;
  _SignedHEX(T v): val(v) {}
};

template<typename T>
inline Print &operator <<(Print &obj, const _SignedHEX<T> &arg)
{ 
  if (arg.val >= 0) obj.print(arg.val, HEX);
  else 
  {
    obj.print("-"); obj.print(-arg.val, HEX);
  } 
  return obj; 
}

#define _WIDTH2Hex(a) _WIDTHZ(_HEX(a), 2)
inline Print &operator <<(Print &obj, const _SignedHEX<CRGB> &arg) { return obj << _WIDTH2Hex(arg.val.r) << _WIDTH2Hex(arg.val.g) << _WIDTH2Hex(arg.val.b); }

#define _HEXS(a)      _SignedHEX<typeof(a)>(a)

// _log << SpaceIt(mpu.axis.x, mpu.axis.y, mpu.axis.z, mpu.angle, mpu.acc, mpu.w) << endl;
// _log << SpaceIt(_HEXS(mpu.axis.x), _HEXS(mpu.axis.y), _HEXS(mpu.axis.z), _HEXS(mpu.angle), _HEXS(mpu.acc), _HEXS(mpu.w)) << endl;

// CRGB c = strtol("FF0000", nullptr, 16);
// CRGB c2 = CRGB::Red;
// _log << SpaceIt(_HEXS(c2), _HEXS(c), c.r, c.g, c.b) << endl; // FF0000 FF0000 255 0 0
// int  v = strtol("-10", nullptr, 16);
// _log << SpaceIt(v, _HEX(v), _HEXS(v)) << endl; // -16 FFFFFFF0 -10
// _log << (CRGB(c2) >= CRGB(0)) << endl;


