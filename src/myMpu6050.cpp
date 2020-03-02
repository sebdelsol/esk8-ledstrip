#include <myMpu6050.h>
#include <I2Cdev.h>
//#include <MPU6050_6Axis_MotionApps20.h>
#include <MPU6050_6Axis_MotionApps_V6_12.h> //better

#define ONEG 8192

MPU6050 mpu;

// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion Q;                     // [w, x, y, z]         quaternion container
VectorInt16 aa;                   // [x, y, z]            accel sensor measurements
VectorInt16 aaReal;               // [x, y, z]            gravity-free accel sensor measurements
VectorFloat gravity;              // [x, y, z]            gravity vector
float ypr[3] = {.0f, .0f, .0f};   // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

//--------------------------------------
void myMPU6050::begin(Stream &serial)
{
  mSerial = &serial;
  
  Wire.begin(SDA, SCL);
  Wire.setClock(400000); // 400kHz I2C clock.

  *mSerial << "Initializing mpu...";
  mpu.initialize();

  // verify connection
  *mSerial << "connection " << (mpu.testConnection() ? F("successful") : F("failed"));// << endl;

  // mpu.PrintActiveOffsets();
  // supply gyro offsets // seems not useful with dmp
  mpu.setXGyroOffset(77);
  mpu.setYGyroOffset(4);
  mpu.setZGyroOffset(8);
  mpu.setXAccelOffset(-1772);
  mpu.setYAccelOffset(-155);
  mpu.setZAccelOffset(1270);

  devStatus = mpu.dmpInitialize();

  if (devStatus == 0) { // did it work ?
    mpu.CalibrateAccel(6);
    mpu.CalibrateGyro(6);
    mpu.PrintActiveOffsets();

    *mSerial << "Enabling DMP...";
    mpu.setDMPEnabled(true);
    mpuIntStatus = mpu.getIntStatus();
    dmpReady = true;

    // get expected DMP packet size for later comparison
    packetSize = mpu.dmpGetFIFOPacketSize();
    *mSerial << "done" << endl;
  }
  else // ERROR! 1 = initial memory load failed, 2 = DMP configuration updates failed
    *mSerial << "DMP Initialization failed (" << devStatus << ")" << endl;

}

// ================================================================
bool myMPU6050::readAccel() {
  if (dmpReady){

    // something available ?
    fifoCount = mpu.getFIFOCount();
    if (fifoCount >= packetSize){

      mpuIntStatus = mpu.getIntStatus();

      // check for overflow
      if ((mpuIntStatus & _BV(MPU6050_INTERRUPT_FIFO_OFLOW_BIT)) || fifoCount >= 1024) {
        mpu.resetFIFO(); // reset so we can continue cleanly
        *mSerial << "FIFO overflow! " << endl;
      }

      // otherwise, check for DMP data ready interrupt
      else if (mpuIntStatus & _BV(MPU6050_INTERRUPT_DMP_INT_BIT)) {
        // read all available
        while (fifoCount >= packetSize){
          mpu.getFIFOBytes(fifoBuffer, packetSize);
          fifoCount -= packetSize;
        }

        // angles
        mpu.dmpGetQuaternion(&Q, fifoBuffer);
        mpu.dmpGetGravity(&gravity, &Q);
        mpu.dmpGetYawPitchRoll(ypr, &Q, &gravity);

        // real acceleration, adjusted to remove gravity
        mpu.dmpGetQuaternion(&Q, fifoBuffer);
        mpu.dmpGetAccel(&aa, fifoBuffer);
        mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);
        return true;
      }
    }
  }
  return false;
}

#define ONEMil 1000000.

bool myMPU6050::getXYZ(float **YPR, float &wz, int &x, int &y, int &z, int &oneG) 
{
  if (readAccel()) {

    ulong t = micros();
    long dt = t - mT;
    mT = t;

    uint16_t smooth = - int(pow(1. - ACCEL_AVG, dt * ACCEL_BASE_FREQ / ONEMil) * 65536.); // 1 - (1-accel_avg) ^ (dt * 60 / 1000) using fract16
    mX = lerp15by16(mX, aaReal.x, smooth);
    mY = lerp15by16(mY, aaReal.y, smooth);
    mZ = lerp15by16(mZ, aaReal.z, smooth);

    mWz = (ypr[0] - mAngz) * ONEMil / dt;
    mAngz = ypr[0];

    // #define MPU_DBG
    #ifdef MPU_DBG
      *mSerial << "[ dt " << dt/1000. << "ms, smooth " << smooth/65536. << ", Wz " << mWz  << "]  ";
      *mSerial << "[ ypr " << ypr[0] * 180/M_PI << "  " << ypr[1] * 180/M_PI << "  " << ypr[2] * 180/M_PI << "]  ";
      *mSerial << "[ grav " << gravity.x << "  " << gravity.y << "  " << gravity.z << "]  ";
      *mSerial << "[ avg " << mX << "  " << mY << "  " << mZ << "]  ";
      *mSerial << "[ acc " << aa.x << "\t" << aa.y << "\t" << aa.z << "]  ";
      *mSerial << "[ real " << aaReal.x << "\t" << aaReal.y << "\t" << aaReal.z << "]  ";
      *mSerial << "                 \r"; //endl;
    #endif
  }

  wz = mWz;
  x = mX;
  y = mY;
  z = mZ;
  *YPR = ypr;
  oneG = ONEG;

  return dmpReady;
}
