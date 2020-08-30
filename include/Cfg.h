#pragma once

// -------------------- main cfg
#define   SERIAL_BAUD   115200  // Hz

#define   LED_MAX_MA    800     // mA, lower Cfg.bright to avoid reaching this value
#define   LED_DITHERING false

#define   LED_TICK      10      // ms, leds update
#define   BT_TICK       30      // ms, bluetooth update
#define   WIFI_TICK     15      // ms, wifi update for OTA, telnet & led debug
#define   MOTION_TICK   10      // ms, MPU internaly update every 10ms

#define   NBLEDS_MIDDLE 72
#define   NBLEDS_TIPS   36

// --------------------- colors
#define   AQUA          CRGB(0x00FFFF)
#define   AQUA_MENTHE   CRGB(0x7FFFD4)
#define   LUSH_LAVA     CRGB(0xFF4500)
#define   HUE_AQUA_BLUE 140

// --------------------- maxs
#define   MAX_uint8     255
#define   MAX_probe     4095
#define   MAX_int16     32767

// --------------------- saved cfg
struct CFG : public OBJVar
{
  // update ?
  bool stripMid   = true;
  bool stripRear  = true;
  bool stripFront = true;

  // brightness ?
  byte bright     = 255;  // half brightness is enough tho to avoid reaching LED_MAX_MA
  int fade        = 0;    // for the fade in
  
  int  minProbe   = 400;
  bool probe      = false;

  byte pacifica   = 158;
  byte fire       = 128;

  // for rotation
  byte runSpeed   = 3;
  int  neutralWZ  = 3000;
  int  maxWZ      = 7000; 

  // for acc
  byte divAcc     = 2;
  int  smoothAcc  = 1600;
  byte thresAcc   = 30;

  // for acc lerping
  int FWD         = 0;
  int RWD         = 0;

  // front & rear Cylons
  byte minEye     = 5;
  byte maxEye     = 10;

  // rear Fire
  int  minDim     = 4;
  int  maxDim     = 10;

  // rear twinkle
  int minTwkR     = 54;

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

    AddBool(stripMid);
    AddBool(stripRear);
    AddBool(stripFront);

    AddBool(probe);
    AddVar (minProbe,   1, MAX_probe);
    AddVar (bright,     1, MAX_uint8);

    AddVar (pacifica,   0, MAX_uint8);
    AddVar (fire,       0, MAX_uint8);

    AddVar (runSpeed,   0, 10);
    AddVar (neutralWZ,  0, MAX_int16);
    AddVar (maxWZ,      0, MAX_int16);

    AddVar (divAcc,     1, 10);
    AddVar (smoothAcc,  1, MAX_int16);
    AddVar (thresAcc,   0, MAX_uint8);

    AddVar (minEye,     1, (NBLEDS_TIPS>>1));
    AddVar (maxEye,     1, (NBLEDS_TIPS>>1));

    AddVar (minDim,     1, 10);
    AddVar (maxDim,     1, 10);
    AddVar (minTwkR,    0, MAX_uint8);
  };
};