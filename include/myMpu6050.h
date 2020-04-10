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
// if you need to compute the offset
// #define MPU_ZERO 
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
#define STAYS_SHORT(x) constrain(x, -32768, 32767)
#define TOdeg(x) (x * 180/M_PI)

//-----------------------------
class myMPU6050
{
  ulong mT = 0;
  int mX = 0, mY = 0, mZ = 0, mWz = 0;
  Stream* mSerial;

  bool mDmpReady = false;           // set true if DMP init was successful
  uint8_t mFifoBuffer[64];          // FIFO storage buffer

  Quaternion mQuat;                 // [w, x, y, z]         quaternion container
  VectorInt16 mGy;                  // [x, y, z]            gyro sensor measurements
  VectorInt16 mAcc;                 // [x, y, z]            accel sensor measurements
  VectorInt16 mAccReal;             // [x, y, z]            gravity-free accel sensor measurements
  VectorFloat mGrav;                // [x, y, z]            gravity vector
  float mYPR[3] = {.0f, .0f, .0f};  // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

  VectorInt16 mUp;                  // [x, y, z]            up vector
  VectorInt16 mDir;                 // [x, y, z]            dir vector

  bool readAccel();
  
public:

  void begin(Stream &serial, void (*handleOta)());
  bool getMotion(float **YPR, VectorInt16 &dir, VectorInt16 &up, VectorInt16 &acc, int &wz);
};
