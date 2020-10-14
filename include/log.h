#pragma once

#include <Streaming.h>
#include <FastledCfg.h>
#include <FastLED.h>

extern Stream& _log;

// -- Stream extension
// _HEXS() for signed hexadecimal print : _HEXS(-16) gives -10 instead of FFFFFFF0 if an int
// _HEXS() works for CRGB too

template<typename T>
struct _SignHEX
{
  T val;
  _SignHEX(T v): val(v) {};
};

// _HEXS def
#define _HEXS(a) _SignHEX<typeof(a)>(a)

// << _HEXS(v) for any type of v
template<typename T>
inline Print &operator <<(Print &obj, const _SignHEX<T> &arg)
{ 
  if (arg.val >= 0) 
    obj.print(arg.val, HEX);
  else 
  { 
    obj.print("-"); 
    obj.print(-arg.val, HEX); 
  } 
  return obj; 
}

// << _HEXS(v) for CRGB v
inline Print &operator <<(Print &obj, const _SignHEX<CRGB> &arg)
{ 
  #define _HEX2(a) _WIDTHZ(_HEX(a), 2) // at least 2 chars
  const CRGB& c = arg.val;
  return obj << _HEX2(c.r) << _HEX2(c.g) << _HEX2(c.b); 
}

// _log << SpaceIt(mpu.axis.x, mpu.axis.y, mpu.axis.z, mpu.angle, mpu.acc, mpu.w) << endl;
// _log << SpaceIt(_HEXS(mpu.axis.x), _HEXS(mpu.axis.y), _HEXS(mpu.axis.z), _HEXS(mpu.angle), _HEXS(mpu.acc), _HEXS(mpu.w)) << endl;
// _log << SpaceItHex(mpu.axis.x, mpu.axis.y, mpu.axis.z, mpu.angle, mpu.acc, mpu.w) << endl;

// CRGB c = strtol("FF0000", nullptr, 16);
// CRGB c2 = CRGB::Red;
// _log << SpaceIt(_HEXS(c2), _HEXS(c), c.r, c.g, c.b) << endl; // FF0000 FF0000 255 0 0
// int  v = strtol("-10", nullptr, 16);
// _log << SpaceIt(v, _HEX(v), _HEXS(v)) << endl; // -16 FFFFFFF0 -10
// _log << (CRGB(c2) >= CRGB(0)) << endl;


