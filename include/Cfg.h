#pragma once

// ----------------------------------------------------
#define   SERIAL_BAUD   115200  // ms
#define   LED_MAX_MA    800     // mA, please check Cfg.bright to avoid reaching this value

#define   LED_TICK      10      // ms, it's used too  for mpu6050 which is refreshed every 10ms
#define   BT_TICK       30      // ms, bluetooth updates

#define   NBLEDS_MIDDLE 72
#define   NBLEDS_TIPS   36

// ----------------------------------------------------
#define   AQUA          CRGB(0x00FFFF)
#define   AQUA_MENTHE   CRGB(0x7FFFD4)
#define   LUSH_LAVA     CRGB(0xFF4500)
#define   HUE_AQUA_BLUE 140

// ----------------------------------------------------
class CFG : public OBJVar
{
public:
  // update ?
  bool stripMid   = true;
  bool stripRear  = true;
  bool stripFront = true;

  // brightness ?
  byte bright     = 255;  // half brightness is enough to avoid reaching LED_MAX_MA
  int fade        = 0;    // for the fade in
  
  #define MaxProbe 4095
  int  minProbe   = 400;
  bool probe      = false;

  byte pacifica   = 158;
  byte fire       = 128;

  // for rotation
  byte runSpeed    = 3;
  int  neutralWZ   = 3000;
  int  maxWZ       = 7000; 

  // for acc
  byte divAcc     = 2;
  int  smoothAcc  = 1600;
  byte thresAcc   = 30;
  
  byte minEye     = 5;
  byte maxEye     = 10;

  // Fire
  int  minDim     = 4;
  int  maxDim     = 10;

  //twinkleR
  int minTwkR     = 54;

  // for lerp
  int FWD = 0;
  int RWD = 0;

#ifdef USE_BT
  AllObjBT&     mAllObj; 
  BlueTooth&    mBT; 
  MOTION&       mMotion;

  CFG(AllObjBT& allObj, BlueTooth& bt, MOTION& motion) : mAllObj(allObj), mBT(bt), mMotion(motion) {};
#endif

  void init()
  {
  #ifdef USE_BT
    REGISTER_CMD       ("save",      mAllObj.save(false)              ) // save not default
    REGISTER_CMD       ("load",      mAllObj.load(false)              ) // load not default
    REGISTER_CMD       ("default",   mAllObj.load(true)               ) // load default
    REGISTER_CMD_NOSHOW("getInits",  mAllObj.sendInits(mBT)           ) // answer with all vars init (min, max, value)
    REGISTER_CMD_NOSHOW("getUpdate", mAllObj.sendUpdate(mBT, mMotion) ) // answer with all updates
  #endif

    REGISTER_VAR_NAME(stripMid,   0, 1);
    REGISTER_VAR_NAME(stripRear,  0, 1);
    REGISTER_VAR_NAME(stripFront, 0, 1);

    REGISTER_VAR_NAME(probe,      0, 1);
    REGISTER_VAR_NAME(minProbe,   1, MaxProbe);
    REGISTER_VAR_NAME(bright,     1, 255);

    REGISTER_VAR_NAME(pacifica,   0, 255);
    REGISTER_VAR_NAME(fire,       0, 255);

    REGISTER_VAR_NAME(runSpeed,   0, 10);
    REGISTER_VAR_NAME(neutralWZ,  0, 32768);
    REGISTER_VAR_NAME(maxWZ,      0, 32768);

    REGISTER_VAR_NAME(divAcc,     1, 10);
    REGISTER_VAR_NAME(smoothAcc,  1, 32768);
    REGISTER_VAR_NAME(thresAcc,   0, 255);

    REGISTER_VAR_NAME(minEye,     1, (NBLEDS_TIPS>>1));
    REGISTER_VAR_NAME(maxEye,     1, (NBLEDS_TIPS>>1));

    REGISTER_VAR_NAME(minDim,     1, 10);
    REGISTER_VAR_NAME(maxDim,     1, 10);
    REGISTER_VAR_NAME(minTwkR,    0, 255);
  };
};
