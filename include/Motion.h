#pragma once

#define FASTLED_ESP32_FLASH_LOCK 1
#include <FastLED.h> // for lerp15by16

#include <Wire.h>
#include <helper_3dmath.h>  // vector & quaternion 
#include <Streaming.h>
#include <Pins.h>
#include <ObjVar.h>

#define MPU6050_INCLUDE_DMP_MOTIONAPPS20 // so that all dmp functions are included
#include <MPU6050.h>

//----------------------------- dmp Version & Debug
// #define USE_V6.12
// #define MPU_DBG

//----------------------------- calibration
#define CALIBRATION_LOOP  6

//----------------------------- Smooth accel & gyro
#define ACCEL_AVG         .05 // use 5% of the new measure in the avg
#define ACCEL_BASE_FREQ   60. // based on a 60fps measure

//----------------------------- Run in a task
#define MPU_GETFIFO_CORE 0 // mpu on a task
#define MPU_GETFIFO_PRIO 1

//----------------------------- 
struct SensorOutput 
{
  VectorInt16 axis;
  int         angle = 0;
  int         accX = 0, accY = 0, accZ = 0;
  int         wZ = 0;
  bool        updated = false;
};

//-----------------------------
class MOTION : public OBJVar, public MPU6050
{
  Stream&     mSerial;
  bool        mDmpReady = false; // if DMP init was successful
  uint8_t*    mFifoBuffer; // FIFO storage buffer

  ulong       mT = 0;
  Quaternion  mQuat;      // quaternion from fifoBuffer
  VectorInt16 mW;         // gyro sensor
  VectorInt16 mAcc;       // accel sensor
  VectorInt16 mAccReal;   // gravity-free accel
  VectorFloat mGrav;      // gravity vector

  int16_t     mXGyroOffset,   mYGyroOffset,   mZGyroOffset;
  int16_t     mXAccelOffset,  mYAccelOffset,  mZAccelOffset;
  bool        gotOffsets  = false;

  void getAxiSAngle(VectorInt16 &v, int &angle, Quaternion &q);
  void printOffsets();
  bool setOffsets();

public:
  SensorOutput  mOutput; // useful outpout

  MOTION(Stream& serial);
  void init();
  void begin();
  void calibrate();
  bool getFiFoPacket() { return dmpGetCurrentFIFOPacket(mFifoBuffer); };
  void compute(SensorOutput& output);
  void update();
};
