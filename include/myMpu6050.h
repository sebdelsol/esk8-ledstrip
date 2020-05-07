#pragma once

#define FASTLED_ESP32_FLASH_LOCK 1
#include <FastLED.h> // for lerp15by16

#include <Wire.h>
#include <Streaming.h>
#include <myPins.h>
#include <helper_3dmath.h>
#include <objVar.h>

//----------------------------- smooth accel & gyro
#define ACCEL_AVG         .05 // use 5% of the new measure in the avg
#define ACCEL_BASE_FREQ   60. // based on a 60fps measure
#define CALIBRATION_LOOP  6

#define STAYS_SHORT(x) constrain(x, -32768, 32767)
#define TOdeg(x) (x * 180/M_PI)

//-----------------------------
// The core to run mpu.dmpGetCurrentFIFOPacket()
// #define MPU_GETFIFO_CORE 0 // mpu on a task
// #define MPU_GETFIFO_PRIO 0
// #define MPU_NB_BadReading 10 // @ beginning

//-----------------------------
class myMPU6050 : public OBJVar
{
  ulong mT = 0;
  int mX = 0, mY = 0, mZ = 0, mWz = 0;

  Stream* mSerial;

  bool mDmpReady = false;           // set true if DMP init was successful

  Quaternion  mQuat;                // [w, x, y, z]         quaternion container
  VectorInt16 mGy;                  // [x, y, z]            gyro sensor measurements
  VectorInt16 mAcc;                 // [x, y, z]            accel sensor measurements
  VectorInt16 mAccReal;             // [x, y, z]            gravity-free accel sensor measurements
  VectorFloat mGrav;                // [x, y, z]            gravity vector
  VectorInt16 mAxis;
  int         mAngle;

  int16_t mXGyroOffset,   mYGyroOffset,   mZGyroOffset;
  int16_t mXAccelOffset,  mYAccelOffset,  mZAccelOffset;

  void getAxiSAngle(VectorInt16 &v, int &angle, Quaternion &q);
  bool getFifoBuf();
  bool readAccel();

  void loadCalibration();
  void calibrate();

public:

  uint16_t mPacketSize;
  uint8_t* mFifoBuffer;          // FIFO storage buffer

  void init();
  void begin(Stream& serial, bool doCalibrate = false);
  bool getMotion(VectorInt16& axis, int& angle, VectorInt16& acc, int& wz);
};
