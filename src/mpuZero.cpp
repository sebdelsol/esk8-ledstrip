#include <myMpu6050.h>

Stream* mSerial;
void (*mHandleOta)();

void SetAveraging(int NewN);

// I2Cdev and MPU6050 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include "I2Cdev.h"
#include "MPU6050.h"

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 accelgyro;
//MPU6050 accelgyro(0x69); // <-- use for AD0 high


const char LBRACKET = '[';
const char RBRACKET = ']';
const char COMMA    = ',';
const char BLANK    = ' ';
const char PERIOD   = '.';

const int iAx = 0;
const int iAy = 1;
const int iAz = 2;
const int iGx = 3;
const int iGy = 4;
const int iGz = 5;

const int usDelay = 3150;   // empirical, to hold sampling to 200 Hz
const int NFast =  1000;    // the bigger, the better (but slower)
const int NSlow = 10000;    // ..
const int LinesBetweenHeaders = 5;
      int LowValue[6];
      int HighValue[6];
      int Smoothed[6];
      int LowOffset[6];
      int HighOffset[6];
      int Target[6];
      int LinesOut;
      int N;

void ForceHeader()
{ 
  LinesOut = 99; 
}

void GetSmoothed()
{ int16_t RawValue[6];
  int i;
  long Sums[6];
  for (i = iAx; i <= iGz; i++)
  { 
    Sums[i] = 0; 
  }
//    unsigned long Start = micros();

  for (i = 1; i <= N; i++)
  { // get sums
    accelgyro.getMotion6(&RawValue[iAx], &RawValue[iAy], &RawValue[iAz],
                          &RawValue[iGx], &RawValue[iGy], &RawValue[iGz]);
    if ((i % 500) == 0)
      mSerial->print(PERIOD);
    delayMicroseconds(usDelay);
    for (int j = iAx; j <= iGz; j++)
      Sums[j] = Sums[j] + RawValue[j];
  } // get sums
//    unsigned long usForN = micros() - Start;
//    mSerial->print(" reading at ");
//    mSerial->print(1000000/((usForN+N/2)/N));
//    mSerial->println(" Hz");
  for (i = iAx; i <= iGz; i++)
  { 
    Smoothed[i] = (Sums[i] + N/2) / N ; 
  }
} // GetSmoothed

void Initialize()
{
  // join I2C bus (I2Cdev library doesn't do this automatically)
  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    Wire.begin(SDA, SCL);
  #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
      Fastwire::setup(400, true);
  #endif

  // initialize device
  mSerial->println("Initializing I2C devices...");
  accelgyro.initialize();

  // verify connection
  mSerial->println("Testing device connections...");
  mSerial->println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");
} // Initialize

void SetOffsets(int TheOffsets[6])
{ 
  accelgyro.setXAccelOffset(TheOffsets [iAx]);
  accelgyro.setYAccelOffset(TheOffsets [iAy]);
  accelgyro.setZAccelOffset(TheOffsets [iAz]);
  accelgyro.setXGyroOffset (TheOffsets [iGx]);
  accelgyro.setYGyroOffset (TheOffsets [iGy]);
  accelgyro.setZGyroOffset (TheOffsets [iGz]);
} // SetOffsets

void ShowProgress()
{ 
  if (LinesOut >= LinesBetweenHeaders)
  { // show header
    mSerial->println("\tXAccel\t\t\tYAccel\t\t\t\tZAccel\t\t\tXGyro\t\t\tYGyro\t\t\tZGyro");
    LinesOut = 0;
  } // show header
  mSerial->print(BLANK);
  for (int i = iAx; i <= iGz; i++) { 
    mSerial->print(LBRACKET);
    mSerial->print(LowOffset[i]),
    mSerial->print(COMMA);
    mSerial->print(HighOffset[i]);
    mSerial->print("] --> [");
    mSerial->print(LowValue[i]);
    mSerial->print(COMMA);
    mSerial->print(HighValue[i]);
    if (i == iGz) { 
      mSerial->println(RBRACKET); 
    }
    else { 
      mSerial->print("]\t"); 
    }
  }
  LinesOut++;
  mHandleOta();
} // ShowProgress

void PullBracketsIn()
{ 
  boolean AllBracketsNarrow;
  boolean StillWorking;
  int NewOffset[6];

  mSerial->println("\nclosing in:");
  AllBracketsNarrow = false;
  ForceHeader();
  StillWorking = true;
  while (StillWorking) { 
    StillWorking = false;
    if (AllBracketsNarrow && (N == NFast)) { 
      SetAveraging(NSlow); 
    }
    else { 
      AllBracketsNarrow = true; 
    }// tentative
    for (int i = iAx; i <= iGz; i++) { 
      if (HighOffset[i] <= (LowOffset[i]+1)) { 
        NewOffset[i] = LowOffset[i]; 
      }
      else { // binary search
        StillWorking = true;
        NewOffset[i] = (LowOffset[i] + HighOffset[i]) / 2;
        if (HighOffset[i] > (LowOffset[i] + 10)) { 
          AllBracketsNarrow = false; 
        }
      } // binary search
    }
    SetOffsets(NewOffset);
    GetSmoothed();
    for (int i = iAx; i <= iGz; i++) {
       // closing in
      if (Smoothed[i] > Target[i]) { // use lower half
        HighOffset[i] = NewOffset[i];
        HighValue[i] = Smoothed[i];
      } // use lower half
      else { // use upper half
        LowOffset[i] = NewOffset[i];
        LowValue[i] = Smoothed[i];
      } // use upper half
    } // closing in
    ShowProgress();
  } // still working

} // PullBracketsIn

void PullBracketsOut()
{ 
  boolean Done = false;
  int NextLowOffset[6];
  int NextHighOffset[6];

  mSerial->println("expanding:");
  ForceHeader();

  while (!Done) { 
    Done = true;
    SetOffsets(LowOffset);
    GetSmoothed();
    for (int i = iAx; i <= iGz; i++) { // got low values
      LowValue[i] = Smoothed[i];
      if (LowValue[i] >= Target[i]) { 
        Done = false;
        NextLowOffset[i] = LowOffset[i] - 1000;
      }
      else { 
        NextLowOffset[i] = LowOffset[i]; 
      }
    } // got low values

    SetOffsets(HighOffset);
    GetSmoothed();
    for (int i = iAx; i <= iGz; i++) { // got high values
      HighValue[i] = Smoothed[i];
      if (HighValue[i] <= Target[i]) { 
        Done = false;
        NextHighOffset[i] = HighOffset[i] + 1000;
      }
      else { 
        NextHighOffset[i] = HighOffset[i]; 
      }
    } // got high values
    ShowProgress();
    for (int i = iAx; i <= iGz; i++) { 
      LowOffset[i] = NextLowOffset[i];   // had to wait until ShowProgress done
      HighOffset[i] = NextHighOffset[i]; // ..
    }
  } // keep going
} // PullBracketsOut

void SetAveraging(int NewN)
{ 
  N = NewN;
  mSerial->print("averaging ");
  mSerial->print(N);
  mSerial->println(" readings each time");
} // SetAveraging

void MPUzero(Stream &serial, void (*handleOta)())
{ 
  mSerial = &serial;
  mHandleOta = handleOta;
  Initialize();

  for (int i = iAx; i <= iGz; i++) { // set targets and initial guesses
    Target[i] = 0; // must fix for ZAccel
    HighOffset[i] = 0;
    LowOffset[i] = 0;
  } // set targets and initial guesses
  Target[iAz] = 16384;
  SetAveraging(NFast);

  PullBracketsOut();
  PullBracketsIn();

  mSerial->println("-------------- done --------------");
} // setup
