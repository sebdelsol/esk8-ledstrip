#pragma once

#include <Wire.h>
#include <Streaming.h>
#include <FastLED.h> // for lerp15by16
#include <myPins.h>
#include <helper_3dmath.h>

//----------------------------- smooth accel & gyro
#define ACCEL_AVG       .05 // use 5% of the new measure in the avg
#define ACCEL_BASE_FREQ 60. // based on a 60fps measure

//----------------------------- OFFSETS

// #define MPU_ZERO // if you need to compute the offset
#ifdef MPU_ZERO
  void MPUzero(Stream &serial, void (*handleOta)());
#endif

#define XGyroOffset   80 //77
#define YGyroOffset   4
#define ZGyroOffset   8
#define XAccelOffset  -1843 // -1772
#define YAccelOffset  -267 // -155
#define ZAccelOffset  1297 // 1270

//-----------------------------
#define TOdeg(x) ( x * 180/M_PI )

class myMPU6050
{
  ulong mT = 0;
  int mX = 0, mY = 0, mZ = 0, mAngz = 0, mWz = 0;
  Stream* mSerial;

  bool readAccel();
  
public:

  void begin(Stream &serial, void (*handleOta)());
  bool getXYZ(float **YPR, int &wz, int &x, int &y, int &z, int &oneG);
};
