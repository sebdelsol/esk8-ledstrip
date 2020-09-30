#pragma once

// -------------------- main cfg
#define   SERIAL_BAUD   115200  // Hz

#define   LED_TICK      10      // ms, leds update
#define   BT_TICK       30      // ms, bluetooth update
#define   WIFI_TICK     30      // ms, wifi update for OTA, telnet & led debug
#define   MPU_TICK      10      // ms, MPU internaly updates every 10ms

#define   NBLEDS_MIDDLE 72
#define   NBLEDS_TIPS   36

// --------------------- colors
#define   AQUA          CRGB(0x00FFFF)
#define   AQUA_MENTHE   CRGB(0x7FFFD4)
#define   LUSH_LAVA     CRGB(0xFF4500)
#define   HUE_AQUA_BLUE 140

// --------------------- maxs
#define   MAX_uint8     255

// --------------------- Tweaks
struct Tweaks : public OBJVar
{
  // update ?
  bool stripMid   = true;
  bool stripRear  = true;
  bool stripFront = true;

  byte pacifica   = 158;
  byte fire       = 128;

  byte runSpeed   = 3;

  // // for acc lerping
  // int FWD         = 0;
  // int RWD         = 0;

  // front & rear Cylons
  byte minEye     = 5;
  byte maxEye     = 10;

  // rear Fire
  byte  minDim    = 4;
  byte  maxDim    = 10;

  // rear twinkle
  byte minTwkR    = 54;

  void init()
  {
    AddBool(stripMid);
    AddBool(stripRear);
    AddBool(stripFront);

    AddVar (pacifica,   0, MAX_uint8);
    AddVar (fire,       0, MAX_uint8);

    AddVar (runSpeed,   0, 10);

    AddVar (minEye,     1, (NBLEDS_TIPS>>1));
    AddVar (maxEye,     1, (NBLEDS_TIPS>>1));

    AddVar (minDim,     1, 10);
    AddVar (maxDim,     1, 10);
    AddVar (minTwkR,    0, MAX_uint8);
  };
};

// --------------------- Cfg Cmds
#ifdef USE_BT
  struct CFG : public OBJVar
  {
    AllObjBT& allObj; 
    BlueTooth& bt; 
    MPU& mpu;
    
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
  };
#else
  struct CFG : public OBJVar
  {
    void init(){};
  };
#endif
