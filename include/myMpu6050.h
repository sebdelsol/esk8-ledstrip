#pragma once

#include <Wire.h>
#include <Streaming.h>
#include <FastLED.h> // for lerp15by16
#include <myPins.h>
#include <helper_3dmath.h>

#define ACCEL_AVG .1
#define ACCEL_BASE_FREQ 60.

class myMPU6050
{
  ulong mT = 0;
  int mX = 0, mY = 0, mZ = 0;
  Stream* mSerial;

  bool readAccel();
  
public:

  void begin(Stream &serial);
  bool getXYZ(float **YPR, int &x, int &y, int &z, int &oneG);
};
