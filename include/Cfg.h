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
struct CFG : public OBJVar
{
  // update ?
  bool stripMid   = true;
  bool stripRear  = true;
  bool stripFront = true;

  // brightness ?
  byte bright     = 255;  // half brightness is enough to avoid reaching LED_MAX_MA
  int fade        = 0;    // for the fade in
  
  #define MaxProbe  4095
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
        AllObjBT& allObj; BlueTooth& bt; MOTION& motion;
    CFG(AllObjBT& allObj, BlueTooth& bt, MOTION& motion) : allObj(allObj), bt(bt), motion(motion) {};
  #endif

  void init()
  {
    #ifdef USE_BT
      AddCmd   ("save",      allObj.save(false)            ) // save not default
      AddCmd   ("load",      allObj.load(false)            ) // load not default
      AddCmd   ("default",   allObj.load(true)             ) // load default
      AddCmdHid("getInits",  allObj.sendInits(bt)          ) // answer with all vars init (min, max, value)
      AddCmdHid("getUpdate", allObj.sendUpdate(bt, motion) ) // answer with all updates
    #endif

    AddVar(stripMid,   0, 1);
    AddVar(stripRear,  0, 1);
    AddVar(stripFront, 0, 1);

    AddVar(probe,      0, 1);
    AddVar(minProbe,   1, MaxProbe);
    AddVar(bright,     1, 255);

    AddVar(pacifica,   0, 255);
    AddVar(fire,       0, 255);

    AddVar(runSpeed,   0, 10);
    AddVar(neutralWZ,  0, 32768);
    AddVar(maxWZ,      0, 32768);

    AddVar(divAcc,     1, 10);
    AddVar(smoothAcc,  1, 32768);
    AddVar(thresAcc,   0, 255);

    AddVar(minEye,     1, (NBLEDS_TIPS>>1));
    AddVar(maxEye,     1, (NBLEDS_TIPS>>1));

    AddVar(minDim,     1, 10);
    AddVar(maxDim,     1, 10);
    AddVar(minTwkR,    0, 255);
  };
};