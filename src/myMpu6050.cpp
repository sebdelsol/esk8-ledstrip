#include <myMpu6050.h>
#include <I2Cdev.h>
#include <MPU6050_6Axis_MotionApps20.h>

#define ONEG            16384

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
void myMPU6050::begin() {
  Wire.begin(SDA, SCL);
  Wire.setClock(400000); // 400kHz I2C clock.

  Serial << "Initializing mpu...";
  mpu.initialize();

  // verify connection
  Serial << (mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed")) << endl;

  // supply gyro offsets
  mpu.setXGyroOffset(79);
  mpu.setYGyroOffset(23);
  mpu.setZGyroOffset(-7);
  mpu.setXAccelOffset(-1599);
  mpu.setYAccelOffset(-4463);
  mpu.setZAccelOffset(1234);

  // Serial << "Initializing DMP..." << endl;
  devStatus = mpu.dmpInitialize();

  if (devStatus == 0) { // did it work ?
    Serial << "Enabling DMP...";
    mpu.setDMPEnabled(true);
    mpuIntStatus = mpu.getIntStatus();
    dmpReady = true;

    // get expected DMP packet size for later comparison
    packetSize = mpu.dmpGetFIFOPacketSize();
    Serial << "done" << endl;
  }
  else // ERROR! 1 = initial memory load failed, 2 = DMP configuration updates failed
    Serial << "DMP Initialization failed (code " << devStatus << ")" << endl;
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
        Serial << "FIFO overflow! " << endl;
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

bool myMPU6050::getXYZ(float **YPR, int &x, int &y, int &z, int &oneG) {
  if (readAccel()) {

    // Serial << "ypr\t" << ypr[0] * 180/M_PI << "\t" << ypr[1] * 180/M_PI << "\t" << ypr[2] * 180/M_PI << "\t";
    // Serial << "areal\t" << aaReal.x/8192. << "\t" << aaReal.y/8192. << "\t" << aaReal.z/8192. << endl;

    ulong t = millis();
    int w = int(pow(ACCEL_AVG, (t - mT) * ACCEL_BASE_FREQ / 1000.) * 65536.);
    mX = lerp15by16(mX, aaReal.x, w);
    mY = lerp15by16(mY, aaReal.y, w);
    mZ = lerp15by16(mZ, aaReal.z, w);
    mT = t;
  }

  x = mX;
  y = mY;
  z = mZ;
  *YPR = ypr;
  oneG = ONEG;

  return dmpReady;
}
