#pragma once

#include <FastledCfg.h>
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

//----------------------------- calibration & I2c clock
#define CALIBRATION_LOOP  6
#define I2C_CLOCK         400000 // 400kHz 

//----------------------------- Smooth accel & gyro
#define ACCEL_AVG         .05 // use 5% of the new measure in the avg
#define ACCEL_BASE_FREQ   60. // based on a 60fps measure

//----------------------------- Run in a task
#define MPU_GET_CORE  1 // mpu on core 1 to prevent ISR hanging
#define MPU_GET_PRIO  1
#define MPU_GET_STACK 2048

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
  bool        mDmpReady = false; 
  bool        mHasBegun = false;

  uint8_t*    mFifoBuffer; 
  Stream&     mSerial;

  ulong       mT = 0;
  Quaternion  mQuat;      // quat from dmp fifobuffer
  VectorInt16 mW;         // gyro 
  VectorInt16 mAcc;       // accel 
  VectorInt16 mAccReal;   // gravity-free accel
  VectorFloat mGrav;      // gravity 

  int16_t     mXGyroOffset,   mYGyroOffset,   mZGyroOffset;
  int16_t     mXAccelOffset,  mYAccelOffset,  mZAccelOffset;
  bool        mGotOffset  = false;
  bool        mAutoCalibrate = false;

  void getAxiSAngle(VectorInt16 &v, int &angle, Quaternion &q);
  void printOffsets(const __FlashStringHelper* txt);
  bool setOffsets();

public:
  SensorOutput  mOutput; // public outpout

  MOTION(Stream& serial) : mSerial(serial) {};
  void init();
  void begin();
  void calibrate();
  bool getFiFoPacket();
  void compute(SensorOutput& output);
  void update();
};
