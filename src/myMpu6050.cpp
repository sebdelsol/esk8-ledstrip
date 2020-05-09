#include <myMpu6050.h>
#include <I2Cdev.h>

#define __PGMSPACE_H_ 1 // pgmsplace.h define PGMSPACE_INCLUDE instead of __PGMSPACE_H
#include <MPU6050_6Axis_MotionApps20.h>
// #include <MPU6050_6Axis_MotionApps_V6_12.h> // longer to init

MPU6050 mpu;

//--------------------------------------
#ifdef MPU_GETFIFO_CORE
  SemaphoreHandle_t   mpuMeasureMutex;
  EventGroupHandle_t  mpuFlagReady;
  TaskHandle_t        mpuNotifyToCalibrate;
  SensorOutput        sharedMotion; //as intermediary so that the mutex is barely taken by MPUGetTask

  void MPUGetTask(void* _myMpu)
  {
    myMPU6050* myMpu = (myMPU6050* )_myMpu;

    for (;;) // forever
    {
      if(ulTaskNotifyTake(pdTRUE, 0)) // pool the the task notification semaphore
        myMpu->calibrate();

      if(mpu.dmpGetCurrentFIFOPacket(myMpu->mFifoBuffer))
      {
        myMpu->computeMotion();

        xSemaphoreTake(mpuMeasureMutex, portMAX_DELAY);
        memcpy(&sharedMotion, &myMpu->mMotion, sizeof(SensorOutput)); 
        xSemaphoreGive(mpuMeasureMutex);

        xEventGroupSetBits(mpuFlagReady, 1);
      }
    }
  }
#endif

//--------------------------------------
void myMPU6050::init()
{
  #define REGISTER_MPU(var) REGISTER_VAR_SIMPLE_NOSHOW(myMPU6050, #var, self->var, -32768, 32767)

  REGISTER_MPU(mXGyroOffset);   REGISTER_MPU(mYGyroOffset);  REGISTER_MPU(mZGyroOffset);
  REGISTER_MPU(mXAccelOffset);  REGISTER_MPU(mYAccelOffset); REGISTER_MPU(mZAccelOffset);

  #ifdef MPU_GETFIFO_CORE
    REGISTER_CMD(myMPU6050, "calibrate",  {xTaskNotifyGive(mpuNotifyToCalibrate);} ) // trigger a calibration
  #else
    REGISTER_CMD(myMPU6050, "calibrate",  {self->calibrate();} ) 
  #endif
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
  *mSerial << "---------" << endl;
  
  Wire.begin(SDA, SCL);
  Wire.setClock(400000); // 400kHz I2C clock.

  mpu.initialize();
  *mSerial << "MPU connection " << (mpu.testConnection() ? "successful" : "failed") << endl;
  uint8_t devStatus = mpu.dmpInitialize();

  if (devStatus == 0) // did it work ?
  { 
    doCalibrate ? calibrate() : loadCalibration();

    mpu.setDMPEnabled(true);
    mDmpReady = true;

    uint16_t packetSize = mpu.dmpGetFIFOPacketSize();
    mFifoBuffer = (uint8_t* )malloc(packetSize * sizeof(uint8_t)); // FIFO storage buffer
    assert (mFifoBuffer!=NULL);

    #ifdef MPU_GETFIFO_CORE
      mpuMeasureMutex = xSemaphoreCreateMutex();
      mpuFlagReady = xEventGroupCreate();
      xTaskCreatePinnedToCore(MPUGetTask, "mpuTask", 2048, this, MPU_GETFIFO_PRIO, &mpuNotifyToCalibrate, MPU_GETFIFO_CORE);  
      *mSerial << "Mpu runs on task on Core " << MPU_GETFIFO_CORE << " with Prio " << MPU_GETFIFO_PRIO << endl;
    #else 
      *mSerial << "Mpu runs on Main Core" << endl;
    #endif

    *mSerial << "DMP enabled" << endl;
  }
  else // ERROR! 1 = initial memory load failed, 2 = DMP configuration updates failed
    *mSerial << "DMP Initialization failed (" << devStatus << ")" << endl;
  
  *mSerial << "---------" << endl;
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
    v.x = q.x * n; v.y = q.y * n; v.z = q.z * n;
  }
}

//--------------------------------------
void myMPU6050::computeMotion()
{
  ulong t = micros();
  ulong dt = t - mT;
  mT = t;

  mpu.dmpGetQuaternion(&mQuat, mFifoBuffer);
  mpu.dmpGetGyro(&mW, mFifoBuffer);

  // axis angle
  mpu.dmpGetGravity(&mGrav, &mQuat);
  getAxiSAngle(mMotion.axis, mMotion.angle, mQuat);

  // real acceleration, adjusted to remove gravity
  mpu.dmpGetAccel(&mAcc, mFifoBuffer);
  mpu.dmpGetLinearAccel(&mAccReal, &mAcc, &mGrav);

  // smooth acc & gyro
  uint16_t smooth = - int(pow(1. - ACCEL_AVG, dt * ACCEL_BASE_FREQ * .000001) * 65536.); // 1 - (1-accel_avg) ^ (dt * 60 / 1000 000) using fract16
  mMotion.accX =  lerp15by16(mMotion.accX,  STAYS_SHORT(mAccReal.x),  smooth);
  mMotion.accY =  lerp15by16(mMotion.accY,  STAYS_SHORT(mAccReal.y),  smooth);
  mMotion.accZ =  lerp15by16(mMotion.accZ,  STAYS_SHORT(mAccReal.z),  smooth);
  mMotion.wZ =    lerp15by16(mMotion.wZ,    STAYS_SHORT(mW.z * -655), smooth);

  // #define MPU_DBG
  #ifdef MPU_DBG
    *mSerial << "[ dt "   << dt*.001 << "ms\t smooth" << smooth/65536. << "\t Wz "  << mMotion.wZ  << "]\t ";
    *mSerial << "[ gyr "  << mW.x << "\t "            << mW.y << "\t "              << mW.z << "\t ";
    *mSerial << "[ grav " << mGrav.x << "\t "         << mGrav.y << "\t "           << mGrav.z << "]\t ";
    *mSerial << "[ avg "  << mMotion.accX << "\t "    << mMotion.accY << "\t "      << mMotion.accZ << "]\t ";
    *mSerial << "[ acc "  << mAcc.x << "\t "          << mAcc.y << "\t "            << mAcc.z << "]\t ";
    *mSerial << "[ real " << mAccReal.x << "\t "      << mAccReal.y << "\t "        << mAccReal.z << "]\t ";
    *mSerial << endl;
  #endif
}

//--------------------------------------
bool myMPU6050::getMotion(SensorOutput& m)
{
#ifdef MPU_GETFIFO_CORE
  if (mDmpReady && xEventGroupGetBits(mpuFlagReady))
  {
    if(xSemaphoreTake(mpuMeasureMutex, 0) == pdTRUE) // pool the mpuTask
    {
      memcpy(&m, &sharedMotion, sizeof(SensorOutput)); 
      xSemaphoreGive(mpuMeasureMutex); // release the mutex after measures have been copied
      return true;
    }
  }
  return false;

#else
  if (mDmpReady)
  {
    if(mpu.dmpGetCurrentFIFOPacket(mFifoBuffer)) 
      computeMotion();
    
    memcpy(&m, &mMotion, sizeof(SensorOutput));
  }
  return mDmpReady;
#endif
}
