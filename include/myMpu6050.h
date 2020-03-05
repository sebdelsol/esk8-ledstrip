#pragma once

#include <Wire.h>
#include <Streaming.h>
#include <FastLED.h> // for lerp15by16
#include <myPins.h>
#include <helper_3dmath.h>

#define ACCEL_AVG .05 //.1
#define ACCEL_BASE_FREQ 60.

#define TOdeg(x) ( x * 180/M_PI )

class myMPU6050
{
  ulong mT = 0;
  int mX = 0, mY = 0, mZ = 0, mAngz = 0, mWz = 0;
  Stream* mSerial;

  bool readAccel();
  
public:

  void begin(Stream &serial);
  bool getXYZ(float **YPR, int &wz, int &x, int &y, int &z, int &oneG);
};
