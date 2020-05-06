#include <myMpu6050.h>
#include <I2Cdev.h>
#include <MPU6050_6Axis_MotionApps20.h>
// #include <MPU6050_6Axis_MotionApps_V6_12.h> // longer to init

MPU6050 mpu;

//--------------------------------------
#ifdef MPU_GETFIFO_CORE
  SemaphoreHandle_t mpuMutex;
  bool mpuReadingCount = 0;

  void MPUGetTask(void* _myMpu)
  {
    myMPU6050* myMpu = (myMPU6050* )_myMpu;
    uint8_t* fifoBuffer = (uint8_t* )malloc(myMpu->mPacketSize * sizeof(uint8_t)); // FIFO storage buffer
    
    for (;;) // forever
    {
      mpu.dmpGetCurrentFIFOPacket(fifoBuffer);
      mpuReadingCount++;;

      xSemaphoreTake(mpuMutex, portMAX_DELAY);
      memcpy(myMpu->mFifoBuffer, fifoBuffer, myMpu->mPacketSize);
      xSemaphoreGive(mpuMutex);
    }
    vTaskDelay( pdMS_TO_TICKS(9) ); // a packet every 10ms 
  }
#endif

//--------------------------------------
void myMPU6050::init()
{
  #define REGISTER_MPU(var) REGISTER_VAR_SIMPLE_NOSHOW(myMPU6050, #var, self->var, -32768, 32767)

  REGISTER_MPU(mXGyroOffset);   REGISTER_MPU(mYGyroOffset);  REGISTER_MPU(mZGyroOffset);
  REGISTER_MPU(mXAccelOffset);  REGISTER_MPU(mYAccelOffset); REGISTER_MPU(mZAccelOffset);

  REGISTER_CMD(myMPU6050, "calibrate",  {self->calibrate();} ) 
}

void myMPU6050::calibrate()
{
  mpu.CalibrateAccel(CALIBRATION_LOOP);
  mpu.CalibrateGyro(CALIBRATION_LOOP);

  mXGyroOffset = mpu.getXGyroOffset();   mYGyroOffset = mpu.getYGyroOffset();   mZGyroOffset = mpu.getZGyroOffset();
  mXAccelOffset = mpu.getXAccelOffset(); mYAccelOffset = mpu.getYAccelOffset(); mZAccelOffset = mpu.getZAccelOffset();

  mpu.PrintActiveOffsets();
}

void myMPU6050::loadCalibration()
{
  mpu.setXGyroOffset(mXGyroOffset);   mpu.setYGyroOffset(mYGyroOffset);   mpu.setZGyroOffset(mZGyroOffset);
  mpu.setXAccelOffset(mXAccelOffset); mpu.setYAccelOffset(mYAccelOffset); mpu.setZAccelOffset(mZAccelOffset); 
  
  mpu.PrintActiveOffsets();
}

//--------------------------------------
void myMPU6050::begin(Stream &serial, bool doCalibrate)
{ 
  mSerial = &serial;
  
  Wire.begin(SDA, SCL);
  Wire.setClock(400000); // 400kHz I2C clock.

  mpu.initialize();
  *mSerial << "MPU connection " << (mpu.testConnection() ? F("successful") : F("failed")) << endl;
  uint8_t devStatus = mpu.dmpInitialize();

  if (devStatus == 0) // did it work ?
  { 
    doCalibrate ? calibrate() : loadCalibration();

    mpu.setDMPEnabled(true);
    mDmpReady = true;

    mPacketSize = mpu.dmpGetFIFOPacketSize();
    mFifoBuffer = (uint8_t *)malloc(mPacketSize * sizeof(uint8_t)); // FIFO storage buffer

    #ifdef MPU_GETFIFO_CORE
      mpuMutex = xSemaphoreCreateMutex();
      xTaskCreatePinnedToCore(MPUGetTask, "mpuTask", 2048, this, MPU_GETFIFO_PRIO, NULL, MPU_GETFIFO_CORE);  
      *mSerial << "Mpu runs on task on Core " << MPU_GETFIFO_CORE << " with Prio " << MPU_GETFIFO_PRIO << endl;
    #endif

    *mSerial << "DMP enabled" << endl;
  }
  else // ERROR! 1 = initial memory load failed, 2 = DMP configuration updates failed
    *mSerial << "DMP Initialization failed (" << devStatus << ")" << endl;
}

//--------------------------------------
void myMPU6050::getAxiSAngle(VectorInt16& v, int& angle, Quaternion& q)
{
  if (q.w > 1) q.normalize(); // needs q.w < 1 for acos and sqrt
  angle = acos(q.w) * 2 * 10430.; // 32767 / PI 
  float s = sqrt(1 - q.w * q.w);
  if (s < 0.001) // div 0
  {
    v.x = 1; v.y = v.z = 0;
  }
  else
  {
    float n = 32767. / s; 
    v.x = q.x * n;
    v.y = q.y * n;
    v.z = q.z * n;
  }
}

//--------------------------------------
#ifdef MPU_GETFIFO_OLD
  bool myMPU6050::getFifoBuf()
  {
    if (mpu.dmpPacketAvailable())
    {
      uint16_t fifoCount = mpu.getFIFOCount();
      
      if (fifoCount >= mPacketSize)
      {
        uint8_t mpuIntStatus = mpu.getIntStatus();

        // check for overflow
        if ((mpuIntStatus & _BV(MPU6050_INTERRUPT_FIFO_OFLOW_BIT)) || fifoCount >= 1024) 
        {
          mpu.resetFIFO(); // reset so we can continue cleanly
          *mSerial << "FIFO overflow! " << endl;
        }

        // otherwise, check for DMP data ready interrupt
        else if (mpuIntStatus & _BV(MPU6050_INTERRUPT_DMP_INT_BIT)) 
        {
          while (fifoCount >= mPacketSize) // read all available
          {
            mpu.getFIFOBytes(mFifoBuffer, mPacketSize);
            fifoCount -= mPacketSize;
          }
          return true;
        }
      }
    }
    return false;
  }
#elif defined(MPU_GETFIFO_CORE)
  bool myMPU6050::getFifoBuf() { return mpuReadingCount > 10 && xSemaphoreTake(mpuMutex, 0) == pdTRUE; } // pool the mpuTask

#else
  bool myMPU6050::getFifoBuf() { return mpu.dmpGetCurrentFIFOPacket(mFifoBuffer); }
#endif

//--------------------------------------
bool myMPU6050::readAccel()
{
  if (mDmpReady && getFifoBuf())
  {
    mpu.dmpGetQuaternion(&mQuat, mFifoBuffer);
    mpu.dmpGetGyro(&mGy, mFifoBuffer);
    mpu.dmpGetAccel(&mAcc, mFifoBuffer);

    #ifdef MPU_GETFIFO_CORE
      xSemaphoreGive(mpuMutex); // release the mutex after mFifoBuffer has been handled
    #endif

    // axis angle
    mpu.dmpGetGravity(&mGrav, &mQuat);
    getAxiSAngle(mAxis, mAngle, mQuat);

    // real acceleration, adjusted to remove gravity
    mpu.dmpGetLinearAccel(&mAccReal, &mAcc, &mGrav);
    
    return true;
  }
  return false;
}

//--------------------------------------
bool myMPU6050::getMotion(VectorInt16& axis, int& angle, VectorInt16& acc, int& wz)
{
  if (readAccel())
  {
    ulong t = micros();
    ulong dt = t - mT;
    mT = t;

    uint16_t smooth = - int(pow(1. - ACCEL_AVG, dt * ACCEL_BASE_FREQ * .000001) * 65536.); // 1 - (1-accel_avg) ^ (dt * 60 / 1000 000) using fract16
    mX =  lerp15by16(mX,  STAYS_SHORT(mAccReal.x),  smooth);
    mY =  lerp15by16(mY,  STAYS_SHORT(mAccReal.y),  smooth);
    mZ =  lerp15by16(mZ,  STAYS_SHORT(mAccReal.z),  smooth);
    mWz = lerp15by16(mWz, STAYS_SHORT(mGy.z * -655),smooth);

    // #define MPU_DBG
    #ifdef MPU_DBG
      *mSerial << "[ gyr " << mWz << "\t " << mGy.x << "\t " << mGy.y << "\t " << mGy.z << "]\t ";
      *mSerial << "[ dt " << dt*.001 << "ms\t smooth" << smooth/65536. << "\t Wz " << mWz  << "]\t ";
      *mSerial << "[ grav " << mGrav.x << "\t " << mGrav.y << "\t " << mGrav.z << "]\t ";
      *mSerial << "[ avg " << mX << "\t " << mY << "\t " << mZ << "]\t ";
      *mSerial << "[ acc " << mAcc.x << "\t " << mAcc.y << "\t " << mAcc.z << "]\t ";
      *mSerial << "[ real " << mAccReal.x << "\t " << mAccReal.y << "\t " << mAccReal.z << "]\t ";
      *mSerial << endl; // *mSerial << "                                 \r"; //endl;
    #endif
  }

  wz = mWz;  
  acc.x = mX;  acc.y = mY;  acc.z = mZ; 
  axis.x = mAxis.x; axis.y = mAxis.y; axis.z = mAxis.z;  
  angle = mAngle;

  return mDmpReady;
}
