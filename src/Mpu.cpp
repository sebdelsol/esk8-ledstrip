#include <mpu.h>

#define __PGMSPACE_H_ 1 // pgmsplace.h define PGMSPACE_INCLUDE instead of __PGMSPACE_H
#ifdef USE_V6_12
  #include <MPU6050_6Axis_MotionApps_V6_12.h> // longer to init & bug with sensitivity
#else
  #include <MPU6050_6Axis_MotionApps20.h>
#endif

//--------------------------------------
#ifdef MPU_GET_CORE
  SemaphoreHandle_t   OutputMutex = xSemaphoreCreateMutex();
  EventGroupHandle_t  FlagReady = xEventGroupCreate();
  TaskHandle_t        NotifyToCalibrate;
  SensorOutput        SharedOutput; // shared with update so that the mutex is barely taken by MPUComputeTask

  void MPUComputeTask(void* _mpu)
  {
    MPU*          mpu = (MPU*) _mpu;
    SensorOutput  taskOutput; //output to store computation
    TickType_t    lastWakeTime = xTaskGetTickCount();

    for (;;) // forever
    {
      if(mpu->getFiFoPacket())
      {
        mpu->compute(taskOutput);

        xSemaphoreTake(OutputMutex, portMAX_DELAY);
        memcpy(&SharedOutput, &taskOutput, sizeof(SensorOutput)); 
        xSemaphoreGive(OutputMutex);

        xEventGroupSetBits(FlagReady, 1);

        if(ulTaskNotifyTake(pdTRUE, 0)) // pool the the task semaphore
          mpu->calibrate();
      }

      // should got a packet every 10ms
      vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(10)); 
    }
  }
#endif

//--------------------------------------
void MPU::init()
{
  // save calibration
  #define AddOffset(var)     AddVarHid(var, -32768, 32767)
  AddOffset(mXGyroOffset);   AddOffset(mYGyroOffset);  AddOffset(mZGyroOffset);
  AddOffset(mXAccelOffset);  AddOffset(mYAccelOffset); AddOffset(mZAccelOffset);
  
  AddBoolNameHid("gotOffset", mGotOffset);
  
  #ifdef MPU_GET_CORE
    xTaskCreatePinnedToCore(MPUComputeTask, "mpuTask", MPU_GET_STACK, this, MPU_GET_PRIO, &NotifyToCalibrate, MPU_GET_CORE);  
    AddCmd("calibrate",  xTaskNotifyGive(NotifyToCalibrate) ) // trigger a calibration
    _log << _FMT(F("Mpu runs on Core % with Prio %"), MPU_GET_CORE, MPU_GET_PRIO) << endl;

  #else
    AddCmd("calibrate",  calibrate() ) 
  #endif

  AddBoolName("auto",       mAutoCalibrate);

  AddVarName ("neutralAcc", mNeutralAcc, 0, 300);
  AddVarName ("divAcc",     mDivAcc,     1, 10);
  AddVarName ("smoothAcc",  mSmoothAcc,  1, 32767)
  AddVarName ("neutralW",   mNeutralW,   0, 32767);
  AddVarName ("maxW",       mMaxW,       0, 32767);
}

//--------------------------------------
void MPU::calibrate()
{
  CalibrateAccel(CALIBRATION_LOOP);
  CalibrateGyro(CALIBRATION_LOOP);

  mXGyroOffset = getXGyroOffset();   mYGyroOffset = getYGyroOffset();   mZGyroOffset = getZGyroOffset();
  mXAccelOffset = getXAccelOffset(); mYAccelOffset = getYAccelOffset(); mZAccelOffset = getZAccelOffset();
  printOffsets(F("MPU calibrated"));
  mGotOffset = true;
}

bool MPU::setOffsets()
{
  _log << F("Try to get Offset...");

  if (mGotOffset)
  {
    setXGyroOffset(mXGyroOffset);   setYGyroOffset(mYGyroOffset);   setZGyroOffset(mZGyroOffset);
    setXAccelOffset(mXAccelOffset); setYAccelOffset(mYAccelOffset); setZAccelOffset(mZAccelOffset); 
    printOffsets(F("Got internal offsets"));
  }
  else 
    _log << endl;

  return mGotOffset;
}

void MPU::printOffsets(const __FlashStringHelper* txt)
{
  _log << txt << endl;
  _log << F("Acc Offset: ") << SpaceIt(_WIDTH(getXAccelOffset(), 6), _WIDTH(getYAccelOffset(), 6), _WIDTH(getZAccelOffset(), 6)) << endl;
  _log << F("Gyr Offset: ") << SpaceIt(_WIDTH(getXGyroOffset(), 6),  _WIDTH(getYGyroOffset(), 6),  _WIDTH(getZGyroOffset(), 6))  << endl;
}

//--------------------------------------
void MPU::begin()
{ 
  mHasBegun = true;
  Wire.begin(SDA, SCL, I2C_CLOCK);

  initialize(); reset(); resetI2CMaster(); //help with startup reliabilily

  _log << F("MPU connection...") << (testConnection() ? F("successful") : F("failed")) << endl;
  uint8_t devStatus = dmpInitialize();

  if (devStatus == 0) // did it work ?
  { 
    if(!mAutoCalibrate || !setOffsets())
      calibrate();

    mFifoBuffer = (uint8_t* )malloc(dmpGetFIFOPacketSize() * sizeof(uint8_t)); // FIFO storage buffer
    assert (mFifoBuffer!=nullptr);

    setDMPEnabled(true);
    mDmpReady = true;
  }
  else // error
  {
    const __FlashStringHelper* error =  devStatus == 1 ? F("initial memory load") : (devStatus == 2 ? F("DMP configuration updates") : F("unknown"));
    _log << _FMT(F("DMP ERROR #% : % failure"), devStatus, error) << endl;
  }
}

//--------------------------------------
void MPU::getAxiSAngle(VectorInt16& v, int& angle, Quaternion& q)
{
  if (q.w > 1) q.normalize(); // needs q.w < 1 for acos and sqrt
  angle = acos(q.w) * 2 * 318.; // 999 / PI 
  
  float s = sqrt(1 - q.w * q.w);
  if (s < 0.001) // div 0
  {
    v.x = 1; v.y = v.z = 0;
  }
  else
  {
    float n = 255. / s; 
    v.x = q.x * n; v.y = q.y * n; v.z = q.z * n;
  }
}

//--------------------------------------
bool MPU::getFiFoPacket() 
{ 
  ulong dt = micros() - mT; // best place to get the actual dt if called right after the delay function

  if (mDmpReady && dmpGetCurrentFIFOPacket(mFifoBuffer))
  {
    mdt = dt;
    mT += dt;
    return true; 
  }
  return false;
}

//--------------------------------------
inline int thresh(const int v, const uint16_t t) 
{
  return v > 0 ? max(0, v - t) : min(v + t, 0);
}

#define _stayShort(x) constrain(x, -32768, 32767)
#define SHIFTR_VECTOR(v, n) v.x = v.x >> n;   v.y = v.y >> n;   v.z = v.z >> n; 

void MPU::compute(SensorOutput& output)
{
  // measures
  dmpGetQuaternion(&mQuat, mFifoBuffer);
  dmpGetGyro(&mW, mFifoBuffer);
  dmpGetAccel(&mAcc, mFifoBuffer);

  #ifdef USE_V6_12 
    // fix sensibility bug in MPU6050_6Axis_MotionApps_V6_12.h
    SHIFTR_VECTOR(mW, 2) 
    SHIFTR_VECTOR(mAcc, 1) 
  #endif

  // gravity & corrected accel
  dmpGetGravity(&mGrav, &mQuat);
  dmpGetLinearAccel(&mAccReal, &mAcc, &mGrav); // remove measured grav

  // smooth acc & gyro
  uint16_t smooth = - int(pow(1. - ACCEL_AVG, mdt * ACCEL_BASE_FREQ * .000001) * 65536.); // 1 - (1-accel_avg) ^ (dt * 60 / 1000 000) using fract16
  mAccY = lerp15by16(mAccY, _stayShort(mAccReal.y),  smooth);
  mWZ   = lerp15by16(mWZ,   _stayShort(mW.z * -655), smooth);

  // output
  getAxiSAngle(output.axis, output.angle, mQuat);

  int16_t acc = _stayShort(thresh(mAccY / mDivAcc, mNeutralAcc) << 8);
  mAccYsmooth = acc * mAccYsmooth < 0 || abs(acc) > abs(mAccYsmooth) ? acc : lerp15by16(mAccYsmooth, acc, mSmoothAcc);
  output.acc = constrain(mAccYsmooth >> 7, -255, 255);
  
  output.w = constrain((thresh(mWZ, mNeutralW) << 8) / mMaxW, -255, 255);
  output.updated = true;

  #ifdef MPU_DBG
    _log << "[ dt "         <<    _WIDTH(mdt * .001, 6) << "ms - smooth " <<      _WIDTH(smooth / 65536.,  6) << "] ";
    _log << "[ smooth acc " <<    _WIDTH(mAccY, 6)      << " - smooth w " <<      _WIDTH(mWZ, 6)              << "] ";
    _log << "[ ouput acc "  <<    _WIDTH(output.acc, 4) << " - ouput w "  <<      _WIDTH(output.w, 4)         << "] ";
    _log << "[ grav " << SpaceIt( _WIDTH(mGrav.x, 5),    _WIDTH(mGrav.y, 5),      _WIDTH(mGrav.z, 5))         << "] ";
    _log << "[ gyr "  << SpaceIt( _WIDTH(mW.x, 4),       _WIDTH(mW.y, 4),         _WIDTH(mW.z, 4))            << "] ";
    _log << "[ acc "  << SpaceIt( _WIDTH(mAcc.x, 6),     _WIDTH(mAcc.y, 6),       _WIDTH(mAcc.z, 6))          << "] ";
    _log << "[ real " << SpaceIt( _WIDTH(mAccReal.x, 6), _WIDTH(mAccReal.y, 6),   _WIDTH(mAccReal.z, 6))      << "] ";
    _log << "\r";
  #endif
}

//--------------------------------------
void MPU::update()
{
  if (!mHasBegun)
    begin();

  #ifdef MPU_GET_CORE
    if (xEventGroupGetBits(FlagReady) && xSemaphoreTake(OutputMutex, 0) == pdTRUE) // pool the mpuTask
    {
      memcpy(&mOutput, &SharedOutput, sizeof(SensorOutput)); 
      xSemaphoreGive(OutputMutex); // release the mutex after measures have been copied
    }
  #else
    if (getFiFoPacket())
      compute(mOutput);
  #endif
}
