#pragma once

#include <Wire.h>
#include <Streaming.h>
#include <FastLED.h> // for lerp15by16
#include <myPins.h>

#define ACCEL_AVG .1
#define ACCEL_BASE_FREQ 60.

class myMPU6050
{
  ulong mT = 0;
  int mX = 0, mY = 0, mZ = 0;

  bool readAccel();
public:

  void begin();
  bool getXYZ(float **YPR, int &x, int &y, int &z, int &oneG);
};
