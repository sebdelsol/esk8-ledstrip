#include <myMpu6050.h>
#include <I2Cdev.h>
#include <MPU6050_6Axis_MotionApps20.h>
// #include <MPU6050_6Axis_MotionApps_V6_12.h> // issue with gravity ?

#define ONEG 8192

MPU6050 mpu;

// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion Q;                     // [w, x, y, z]         quaternion container
VectorInt16 aa;                   // [x, y, z]            accel sensor measurements
VectorInt16 aaReal;               // [x, y, z]            gravity-free accel sensor measurements
VectorFloat gravity;              // [x, y, z]            gravity vector
float ypr[3] = {.0f, .0f, .0f};   // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

//--------------------------------------
void myMPU6050::begin(Stream &serial, void (*handleOta)())
{ 
  #ifdef MPU_ZERO
    MPUzero(serial, handleOta);
  #endif

  mSerial = &serial;
  
  Wire.begin(SDA, SCL);
  Wire.setClock(400000); // 400kHz I2C clock.

  *mSerial << "Initializing mpu...";
  mpu.initialize();
  *mSerial << "connection " << (mpu.testConnection() ? F("successful") : F("failed"));
  devStatus = mpu.dmpInitialize();

  // supply accel & gyro offsets, use #define MPU_ZERO for computing the offsets
  mpu.setXGyroOffset(XGyroOffset);    mpu.setYGyroOffset(YGyroOffset);    mpu.setZGyroOffset(ZGyroOffset);
  mpu.setXAccelOffset(XAccelOffset);  mpu.setYAccelOffset(YAccelOffset);  mpu.setZAccelOffset(ZAccelOffset); 

  if (devStatus == 0) { // did it work ?
    mpu.CalibrateAccel(6);
    mpu.CalibrateGyro(6);
    mpu.PrintActiveOffsets();

    *mSerial << "Enabling DMP...";
    mpu.setDMPEnabled(true);
    dmpReady = true;
    *mSerial << "done" << endl;
  }
  else // ERROR! 1 = initial memory load failed, 2 = DMP configuration updates failed
    *mSerial << "DMP Initialization failed (" << devStatus << ")" << endl;
}

// ================================================================
bool myMPU6050::readAccel()
{
  if (dmpReady && mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) {
    // angles
    mpu.dmpGetQuaternion(&Q, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &Q);
    mpu.dmpGetYawPitchRoll(ypr, &Q, &gravity);

    // real acceleration, adjusted to remove gravity
    mpu.dmpGetAccel(&aa, fifoBuffer);
    mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);
    return true;
  }
  return false;
}

bool myMPU6050::getXYZ(float **YPR, int &wz, int &x, int &y, int &z, int &oneG) 
{
  if (readAccel()) {

    #define STAYS_SHORT(x) constrain(x, -32768, 32767)
    #define TOdeg(x) (x * 180/M_PI)

    ulong t = millis();
    long dt = t - mT;
    mT = t;

    uint16_t smooth = - int(pow(1. - ACCEL_AVG, dt * ACCEL_BASE_FREQ / 1000.) * 65536.); // 1 - (1-accel_avg) ^ (dt * 60 / 1000) using fract16
    mX = lerp15by16(mX, STAYS_SHORT(aaReal.x), smooth);
    mY = lerp15by16(mY, STAYS_SHORT(aaReal.y), smooth);
    mZ = lerp15by16(mZ, STAYS_SHORT(aaReal.z), smooth);

    int cAngz = 65536 * ypr[0];
    int wz = STAYS_SHORT((cAngz - mAngz) * 1000 / dt);
    mWz = lerp15by16(mWz, wz, smooth);
    mAngz = cAngz;

    // #define MPU_DBG
    #ifdef MPU_DBG
      *mSerial << "[ dt " << dt << "ms\t smooth" << smooth/65536. << "\t Wz " << mWz  << "]\t ";
      *mSerial << "[ ypr " << TOdeg(ypr[0]) << "\t " << TOdeg(ypr[1]) << "\t " << TOdeg(ypr[2]) << "]\t ";
      *mSerial << "[ grav " << gravity.x << "\t " << gravity.y << "\t " << gravity.z << "]\t ";
      *mSerial << "[ avg " << mX << "\t " << mY << "\t " << mZ << "]\t ";
      *mSerial << "[ acc " << aa.x << "\t " << aa.y << "\t " << aa.z << "]\t ";
      *mSerial << "[ real " << aaReal.x << "\t " << aaReal.y << "\t " << aaReal.z << "]\t ";
      *mSerial << endl; // *mSerial << "                                 \r"; //endl;
    #endif
  }

  wz = mWz;  x = mX;  y = mY;  z = mZ; 
  *YPR = ypr;  oneG = ONEG;

  return dmpReady;
}
