#include <adxl345.h>

#ifdef SCAN_I2C
  char* myADXL345::getHex(const byte v)
  {
    static char txt[5];
    snprintf(txt, sizeof(txt), "0x%02x", v);
    return txt;
  }

  void myADXL345::scanI2C()
  {
    Wire.begin(SDA, SCL);
    Serial << "I2C Scan ports SDA(" << SDA << ") SCL(" << SCL << ")" << endl;

    int nDevices = 0;

    for (byte addr = 1; addr < 127; addr++) {
      Wire.beginTransmission(addr);
      byte error = Wire.endTransmission(); // 0:success, 1:data too long to fit in transmit buffer, 2:received NACK on transmit of address, 3:received NACK on transmit of data, 4:other error

      if (!error) {
        Serial << "I2C device found @ " << getHex(addr) << endl;
        nDevices++;
      }
      else if (error == 4)
        Serial << "Unknown error @ " << getHex(addr) << endl;
    }

    Serial << (nDevices ? "Done" : "NO I2C devices found") << endl;
  }
#endif

void myADXL345::writeTo(byte address, byte val) {
  Wire.beginTransmission(ADXL345_DEVICE);
  Wire.write(address);
  Wire.write(val);
  Wire.endTransmission();
}

void myADXL345::readFrom(byte address, int num, byte buff[]) {
  Wire.beginTransmission(ADXL345_DEVICE);
  Wire.write(address);
  Wire.endTransmission();

  Wire.beginTransmission(ADXL345_DEVICE);
  Wire.requestFrom(ADXL345_DEVICE, num);  // Request 6 Bytes
  for (int i=0; i<num; i++)
  	if(Wire.available())
  		buff[i] = Wire.read();				// Receive Byte
  Wire.endTransmission();
}

void myADXL345::begin() {
  Wire.begin(); // SDA = D2, SCL = D1 by default

  writeTo(ADXL345_POWER_CTL, 0);	// Wakeup
  writeTo(ADXL345_POWER_CTL, 16);	// Auto_Sleep
  writeTo(ADXL345_POWER_CTL, 8);	// Measure

  byte id;
  readFrom(ADXL345_DEVID, 1, &id);
  // Serial << id << endl;
  mOk = id==ADXL345_GOOD_ID;	// check , it's the right ID

  if (mOk) {
    writeTo(ADXL345_DATA_FORMAT, ADXL345_16G);
    writeTo(ADXL345_BW_RATE, ADXL345_BW_100);
  }
  #ifdef SCAN_I2C
  else
    scanI2C();
  #endif

  Serial << "ADXL345" << (mOk ? " ": " not ") << "initialized" << endl;
}

void myADXL345::readAccel(int &x, int &y, int &z) {
  readFrom(ADXL345_DATAX0, ADXL345_TO_READ, mBuff);	// Read Accel Data from ADXL345
  x = (int16_t)((((int)mBuff[1]) << 8) | mBuff[0]);
  y = (int16_t)((((int)mBuff[3]) << 8) | mBuff[2]);
  z = (int16_t)((((int)mBuff[5]) << 8) | mBuff[4]);
}

#ifdef DO_CALIBRATE
  void myADXL345::calibrate(int ax, int ay, int az)
  {
    _mx = min(ax, _mx);    _my = min(ay, _my);    _mz = min(az, _mz);
    _MX = max(ax, _MX);    _MY = max(ay, _MY);    _MZ = max(az, _MZ);
    int offx = (_mx+_MX)/2, offy = (_my+_MY)/2, offz = (_mz+_MZ)/2;
    _oneG = max (abs(ax-offx), _oneG);

    Serial.printf("ONEG %5d, ", _oneG);
    Serial.printf("OFFSETX %5d, ", offx);
    Serial.printf("OFFSETY %5d, ", offy);
    Serial.printf("OFFSETZ %5d \r", offz);
  }
#endif

#define OFFSETX -1
#define OFFSETY 103
#define OFFSETZ -1533
#define ONEG 320

bool myADXL345::getXYZ(int &x, int &y, int &z, int &oneG)
{
  if (mOk) {
    int ax, ay, az;
    readAccel(ax, ay, az);
    #ifdef DO_CALIBRATE\^
      calibrate(ax, ay, az);
    #endif

    ulong t = millis();
    int w = int(pow(ACCEL_AVG, (t - mT) * ACCEL_BASE_FREQ / 1000.) * 65536.);
    x = mX = lerp15by16(mX, ax + OFFSETX, w);
    y = mY = lerp15by16(mY, ay + OFFSETY, w);
    z = mZ = lerp15by16(mZ, az + OFFSETZ, w);
    mT = t;

    oneG = ONEG;
  }
  return mOk;
}
