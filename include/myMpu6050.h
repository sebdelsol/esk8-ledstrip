#pragma once

#define FASTLED_ESP32_FLASH_LOCK 1
#include <FastLED.h> // for lerp15by16

#include <Wire.h>
#include <Streaming.h>
#include <myPins.h>
#include <helper_3dmath.h>
#include <objVar.h>

#define MPU6050_INCLUDE_DMP_MOTIONAPPS20 // so that all dmp functions are included
#include <MPU6050.h>

//-----------------------------
// #define USE_V6.12
// #define MPU_DBG

//----------------------------- smooth accel & gyro
#define ACCEL_AVG         .05 // use 5% of the new measure in the avg
#define ACCEL_BASE_FREQ   60. // based on a 60fps measure
#define CALIBRATION_LOOP  6

//-----------------------------
// The core to run mpu.dmpGetCurrentFIFOPacket()
#define MPU_GETFIFO_CORE 0 // mpu on a task
#define MPU_GETFIFO_PRIO 1

struct SensorOutput 
{
  int         accX = 0; 
  int         accY = 0; 
  int         accZ = 0;
  int         wZ = 0;
  VectorInt16 axis;
  int         angle = 0;
};

//-----------------------------
class myMPU6050 : public OBJVar, public MPU6050
{
  Stream* mSerial;

  bool mDmpReady = false; // if DMP init was successful

  Quaternion  mQuat;      // quaternion from fifoBuffer
  VectorInt16 mW;         // gyro sensor
  VectorInt16 mAcc;       // accel sensor
  VectorInt16 mAccReal;   // gravity-free accel
  VectorFloat mGrav;      // gravity vector

  int16_t mXGyroOffset,   mYGyroOffset,   mZGyroOffset;
  int16_t mXAccelOffset,  mYAccelOffset,  mZAccelOffset;

  ulong mT = 0;

  void getAxiSAngle(VectorInt16 &v, int &angle, Quaternion &q);
  void loadCalibration();

public:

  // MPU6050       mpu;
  uint8_t*      mFifoBuffer; // FIFO storage buffer
  SensorOutput  mOutput;     // computed motion outpout
  bool          updated = false;

  void init();
  void begin(Stream& serial, bool doCalibrate = false);
  void calibrate();
  void computeMotion(SensorOutput& output);
  void updateMotion();
};
