#include <myMpu6050.h>
#include <I2Cdev.h>
#include <MPU6050_6Axis_MotionApps20.h>

#define ONEG 7780 //8192

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

bool gravityOK = false;
long startTime;

//--------------------------------------
void myMPU6050::begin() {
  Wire.begin(SDA, SCL);
  Wire.setClock(400000); // 400kHz I2C clock.

  Serial << "Initializing mpu...";
  mpu.initialize();

  // verify connection
  Serial << "connection " << (mpu.testConnection() ? F("successful") : F("failed"));// << endl;

  // mpu.PrintActiveOffsets();
  // supply gyro offsets // seems not useful with dmp
  mpu.setXGyroOffset(77);
  mpu.setYGyroOffset(4);
  mpu.setZGyroOffset(8);
  mpu.setXAccelOffset(-1772);
  mpu.setYAccelOffset(-155);
  mpu.setZAccelOffset(1270);
  mpu.PrintActiveOffsets(); 

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
    Serial << "DMP Initialization failed (" << devStatus << ")" << endl;

  startTime = millis();
}

// ================================================================

#define GRAV_CONV_DURATION 15000 //time for gravity to be correct ?
void myMPU6050::dmpGetLinearAccel(VectorInt16 *v, VectorInt16 *vRaw, VectorFloat *gravity)
{
    if (!gravityOK){
      v -> x = vRaw -> x;
      v -> y = vRaw -> y;
      v -> z = vRaw -> z;
      if (millis() - startTime > GRAV_CONV_DURATION) 
        gravityOK = true;
    }
    else {
      // get rid of the gravity component (+1g = +8192 in standard DMP FIFO packet, sensitivity is 2g)
      v -> x = vRaw -> x - gravity -> x * ONEG;
      v -> y = vRaw -> y - gravity -> y * ONEG;
      v -> z = vRaw -> z - gravity -> z * ONEG;
    }
}

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
        dmpGetLinearAccel(&aaReal, &aa, &gravity);
        return true;
      }
    }
  }
  return false;
}

bool myMPU6050::getXYZ(float **YPR, int &x, int &y, int &z, int &oneG) {
  if (readAccel()) {

    ulong t = micros();
    long dt = t - mT;
    mT = t;

    uint16_t w = - int(pow(1. - ACCEL_AVG, dt * ACCEL_BASE_FREQ / 1000000.) * 65536.); // 1 - (1-accel_avg) ^ (dt * 60 / 1000) using fract16
    mX = lerp15by16(mX, aaReal.x, w);
    mY = lerp15by16(mY, aaReal.y, w);
    mZ = lerp15by16(mZ, aaReal.z, w);

    // Serial << "[ grav OK  " << dt/1000. << "ms, w=" << w/65536. << "]  ";;
    // Serial << "[ dt  " << dt/1000. << "ms, w=" << w/65536. << "]  ";;
    // Serial << "[ ypr  " << ypr[0] * 180/M_PI << "  " << ypr[1] * 180/M_PI << "  " << ypr[2] * 180/M_PI << "]  ";
    // Serial << "[ grav  " << (gravityOK ? "OK":"NOT ok") << "\t" << gravity.x << "  " << gravity.y << "  " << gravity.z << "]  ";
    // Serial << "[ AVG  " << mX << "  " << mY << "  " << mZ << "]  ";
    // Serial << "[ aa  " << aa.x << "\t" << aa.y << "\t" << aa.z << "]  "; 
    // Serial << "[ areal  " << aaReal.x << "\t" << aaReal.y << "\t" << aaReal.z << "]  ";
    // Serial << "\r"; //endl;
  }

  x = mX;
  y = mY;
  z = mZ;
  *YPR = ypr;
  oneG = ONEG;

  return dmpReady;
}
