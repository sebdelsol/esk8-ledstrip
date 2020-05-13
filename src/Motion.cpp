#include <Motion.h>

#define __PGMSPACE_H_ 1 // pgmsplace.h define PGMSPACE_INCLUDE instead of __PGMSPACE_H
#ifdef USE_V6.12
  #include <MPU6050_6Axis_MotionApps_V6_12.h> // longer to init & bug with sensitivity
#else
  #include <MPU6050_6Axis_MotionApps20.h>
#endif

//--------------------------------------
#ifdef MPU_GETFIFO_CORE
  SemaphoreHandle_t   mpuOutputMutex;
  EventGroupHandle_t  mpuFlagReady;
  TaskHandle_t        mpuNotifyToCalibrate;
  SensorOutput        sharedOutput; // shared with update so that the mutex is barely taken by MPUGetTask

  void MPUGetTask(void* _myMpu)
  {
    MOTION*    myMpu = (MOTION* )_myMpu;
    SensorOutput  taskOutput; //output to store computation
    long          lastLoop = micros();

    vTaskDelay( pdMS_TO_TICKS(1000) ); // or issue with offset !?

    for (;;) // forever
    {
      // add a delay to share more the core // a packet every 10 ms 
      long ct = micros();
      long dt = ct - lastLoop; lastLoop = ct;
      long wait = 10 - (1 + dt/1000); 
      vTaskDelay( pdMS_TO_TICKS(wait > 0 ? wait : 0) ); // a packet every 10ms 
      
      if(ulTaskNotifyTake(pdTRUE, 0)) // pool the the task notification semaphore
        myMpu->calibrate();

      if(myMpu->getFiFoPacket())
      {
        myMpu->compute(taskOutput);

        xSemaphoreTake(mpuOutputMutex, portMAX_DELAY);
        memcpy(&sharedOutput, &taskOutput, sizeof(SensorOutput)); 
        xSemaphoreGive(mpuOutputMutex);

        xEventGroupSetBits(mpuFlagReady, 1);
      }
    }
  }
#endif

//--------------------------------------

MOTION::MOTION(Stream& serial) : mSerial(serial) {}

void MOTION::init()
{
  #define REGISTER_MPU(var) REGISTER_VAR_SIMPLE_NOSHOW(MOTION, #var, self->var, -32768, 32767)

  REGISTER_MPU(mXGyroOffset);   REGISTER_MPU(mYGyroOffset);  REGISTER_MPU(mZGyroOffset);
  REGISTER_MPU(mXAccelOffset);  REGISTER_MPU(mYAccelOffset); REGISTER_MPU(mZAccelOffset);
  REGISTER_VAR_SIMPLE_NOSHOW(MOTION, "gotOffsets", self->gotOffsets, 0, 1);

  #ifdef MPU_GETFIFO_CORE
    REGISTER_CMD(MOTION, "calibrate",  {xTaskNotifyGive(mpuNotifyToCalibrate);} ) // trigger a calibration
  #else
    REGISTER_CMD(MOTION, "calibrate",  {self->calibrate();} ) 
  #endif
}

void MOTION::printOffsets()
{
  mSerial << "Acc Offset: x " << getXAccelOffset() << "\t y " << getYAccelOffset() << "\t z " << getZAccelOffset() << endl;
  mSerial << "Gyr Offset: x " << getXGyroOffset()  << "\t y " << getYGyroOffset()  << "\t z " << getZGyroOffset()  << endl;
}

void MOTION::calibrate()
{
  CalibrateAccel(CALIBRATION_LOOP);
  CalibrateGyro(CALIBRATION_LOOP);

  mXGyroOffset = getXGyroOffset();   mYGyroOffset = getYGyroOffset();   mZGyroOffset = getZGyroOffset();
  mXAccelOffset = getXAccelOffset(); mYAccelOffset = getYAccelOffset(); mZAccelOffset = getZAccelOffset();
  gotOffsets = true;
  printOffsets();
}

bool MOTION::setOffsets()
{
  if (gotOffsets)
  {
    setXGyroOffset(mXGyroOffset);   setYGyroOffset(mYGyroOffset);   setZGyroOffset(mZGyroOffset);
    setXAccelOffset(mXAccelOffset); setYAccelOffset(mYAccelOffset); setZAccelOffset(mZAccelOffset); 
    printOffsets();
  }
  return gotOffsets;
}

//--------------------------------------
void MOTION::begin()
{ 
  Wire.begin(SDA, SCL);
  Wire.setClock(I2C_CLOCK); 

  initialize();
  mSerial << "MPU connection " << (testConnection() ? "successful" : "failed") << endl;
  uint8_t devStatus = dmpInitialize();

  if (devStatus == 0) // did it work ?
  { 
    if(!setOffsets())
      calibrate();

    setDMPEnabled(true);
    mDmpReady = true;

    uint16_t packetSize = dmpGetFIFOPacketSize();
    mFifoBuffer = (uint8_t* )malloc(packetSize * sizeof(uint8_t)); // FIFO storage buffer
    assert (mFifoBuffer!=NULL);

    #ifdef MPU_GETFIFO_CORE
      mpuOutputMutex = xSemaphoreCreateMutex();
      mpuFlagReady = xEventGroupCreate();
      xTaskCreatePinnedToCore(MPUGetTask, "mpuTask", 2048, this, MPU_GETFIFO_PRIO, &mpuNotifyToCalibrate, MPU_GETFIFO_CORE);  
      mSerial << "Mpu runs in a task on Core " << MPU_GETFIFO_CORE << " with Prio " << MPU_GETFIFO_PRIO << endl;
    #endif

    mSerial << "DMP enabled" << endl;
  }
  else // ERROR! 1 = initial memory load failed, 2 = DMP configuration updates failed
    mSerial << "DMP Initialization failed (" << devStatus << ")" << endl;
}

//--------------------------------------
void MOTION::getAxiSAngle(VectorInt16& v, int& angle, Quaternion& q)
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
#define STAYS_SHORT(x) constrain(x, -32768, 32767)
#define SHIFTR_VECTOR(v, n) v.x = v.x >> n;   v.y = v.y >> n;   v.z = v.z >> n; 

void MOTION::compute(SensorOutput& output)
{
  ulong t = micros();
  ulong dt = t - mT;
  mT = t;

  dmpGetQuaternion(&mQuat, mFifoBuffer);
  dmpGetGyro(&mW, mFifoBuffer);
  #ifdef USE_V6.12
    SHIFTR_VECTOR(mW, 2) // fix sensibility bug in MPU6050_6Axis_MotionApps_V6_12.h
  #endif 

  // axis angle
  dmpGetGravity(&mGrav, &mQuat);
  getAxiSAngle(output.axis, output.angle, mQuat);

  // real acceleration, adjusted to remove gravity
  dmpGetAccel(&mAcc, mFifoBuffer);
  #ifdef USE_V6.12 
    SHIFTR_VECTOR(mAcc, 1) // fix sensibility bug in MPU6050_6Axis_MotionApps_V6_12.h
  #endif
  dmpGetLinearAccel(&mAccReal, &mAcc, &mGrav);

  // smooth acc & gyro
  uint16_t smooth = - int(pow(1. - ACCEL_AVG, dt * ACCEL_BASE_FREQ * .000001) * 65536.); // 1 - (1-accel_avg) ^ (dt * 60 / 1000 000) using fract16
  output.accX =  lerp15by16(output.accX,  STAYS_SHORT(mAccReal.x),  smooth);
  output.accY =  lerp15by16(output.accY,  STAYS_SHORT(mAccReal.y),  smooth);
  output.accZ =  lerp15by16(output.accZ,  STAYS_SHORT(mAccReal.z),  smooth);
  output.wZ =    lerp15by16(output.wZ,    STAYS_SHORT(mW.z * -655), smooth);
  output.updated = true;

  #ifdef MPU_DBG
    mSerial << "[ dt "   << dt*.001 << "ms\t smooth" << smooth/65536. << "\t Wz "  << output.wZ  << "]\t ";
    mSerial << "[ gyr "  << mW.x << "\t "            << mW.y << "\t "              << mW.z << "\t ";
    mSerial << "[ grav " << mGrav.x << "\t "         << mGrav.y << "\t "           << mGrav.z << "]\t ";
    mSerial << "[ avg "  << output.accX << "\t "     << output.accY << "\t "       << output.accZ << "]\t ";
    mSerial << "[ acc "  << mAcc.x << "\t "          << mAcc.y << "\t "            << mAcc.z << "]\t ";
    mSerial << "[ real " << mAccReal.x << "\t "      << mAccReal.y << "\t "        << mAccReal.z << "]\t ";
    mSerial << endl;
  #endif
}

//--------------------------------------
void MOTION::update()
{
#ifdef MPU_GETFIFO_CORE
  
  if (mDmpReady && xEventGroupGetBits(mpuFlagReady) && xSemaphoreTake(mpuOutputMutex, 0) == pdTRUE) // pool the mpuTask
  {
    memcpy(&mOutput, &sharedOutput, sizeof(SensorOutput)); 
    xSemaphoreGive(mpuOutputMutex); // release the mutex after measures have been copied
  }

#else
  
  if (mDmpReady && getFiFoPacket())
    compute(mOutput);

#endif
}
