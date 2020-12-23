#pragma once

// ---- main cfg
#define   SERIAL_BAUD   115200  // Hz

#define   LED_TICK      10      // ms, leds update
#define   BT_TICK       30      // ms, bluetooth update
#define   WIFI_TICK     30      // ms, wifi update for OTA, telnet & led debug
#define   MPU_TICK      10      // ms, MPU internaly updates every 10ms

#define   NLED_MID      72
#define   NLED_TIP      36

// ---- colors
#define   AQUA          CRGB(0x00FFFF)
#define   AQUA_MENTHE   CRGB(0x7FFFD4)
#define   LUSH_LAVA     CRGB(0xFF4500)
#define   HUE_AQUA_BLUE 140

// ---- Tweaks
struct Tweaks : public OBJVar
{
  // update ?
  bool stripMid;
  bool stripRear;
  bool stripFront;

  byte pacifica;
  byte fire;

  byte runSpeed;

  // front & rear Cylons
  byte minEye;
  byte maxEye;

  // rear Fire & twinkle
  byte  minDim;
  byte  maxDim;
  byte minTwkR;

  void init()
  {
    AddBool(stripMid,   true);
    AddBool(stripRear,  true);
    AddBool(stripFront, true);

    AddVar (pacifica,   158,  0, 255);
    AddVar (fire,       128,  0, 255);

    AddVar (runSpeed,   3,    0, 10);

    AddVar (minEye,     5,    1, NLED_TIP >> 1);
    AddVar (maxEye,     10,   1, NLED_TIP >> 1);

    AddVar (minDim,     4,    1, 10);
    AddVar (maxDim,     10,   1, 10);
    AddVar (minTwkR,    54,   0, 255);
  };
};

// ---- Cmds
struct CFG : public OBJVar
{
  #ifdef USE_BT
    AllObjBT&  allObj; 
    BlueTooth& bt; 
    MPU&       mpu;
    
    CFG(AllObjBT& allObj, BlueTooth& bt, MPU& mpu) : allObj(allObj), bt(bt), mpu(mpu) {};

    void init()
    {
      AddCmd   ("save",      allObj.save(CfgType::Current) ) // save not default
      AddCmd   ("load",      allObj.load(CfgType::Current) ) // load not default
      AddCmd   ("default",   allObj.load(CfgType::Default) ) // load default
      AddCmd   ("reset",     ESP.restart()                 ) // reset
      AddCmdHid("getInits",  allObj.sendInits(bt)          ) // answer with all vars init (min, max, value)
      AddCmdHid("getUpdate", allObj.sendUpdate(bt, mpu)    ) // answer with all updates
    };
  #else
    void init(){};
  #endif
};
