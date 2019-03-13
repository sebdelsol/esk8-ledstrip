#pragma once

#include <Wire.h>
#include <Streaming.h>
#include <FastLED.h> // for lerp15by16

// #define DO_CALIBRATE
// #define SCAN_I2C
#define ACCEL_AVG .9
#define ACCEL_BASE_FREQ 60.

#define ADXL345_DEVID			  0x00		// Device ID
#define ADXL345_BW_RATE			0x2C		// Data Rate and Power mode Control
#define ADXL345_POWER_CTL   0x2D		// Power-Saving Features Control
#define ADXL345_DATA_FORMAT 0x31		// Data Format Control
#define ADXL345_DATAX0			0x32		// X-Axis Data 0

#define ADXL345_BW_400			0xD			// 1101		IDD = 140uA
#define ADXL345_BW_200			0xC			// 1100		IDD = 140uA
#define ADXL345_BW_100			0xB			// 1011		IDD = 140uA
#define ADXL345_BW_50			  0xA			// 1010		IDD = 140uA
#define ADXL345_BW_25			  0x9			// 1001		IDD = 90uA

#define ADXL345_16G         B00001011

#define ADXL345_GOOD_ID     0xE5    // Device Id
#define ADXL345_DEVICE      0x53    // Device Address
#define ADXL345_TO_READ     6       // 2 Bytes Per Axis

class myADXL345
{
  #ifdef DO_CALIBRATE
    int _oneG = 0;
    int _mx = 32000, _my = 32000, _mz = 32000;
    int _MX = -32000, _MY = -32000, _MZ = -32000;
    void calibrate(int ax, int ay, int az);
  #endif

  #ifdef SCAN_I2C
    char* getHex(const byte v);
    void scanI2C();
  #endif

  bool mOk;
  ulong mT = 0;
  int mX = 0, mY = 0, mZ = 0;
  byte mBuff[ADXL345_TO_READ];

  void writeTo(byte address, byte val);
  void readFrom(byte address, int num, byte _buff[]);
  void readAccel(int &x, int &y, int &z);
public:

  void begin();
  bool getXYZ(int &x, int &y, int &z, int &oneG);
};
