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

  void init()
  {
    #define REGISTER_CFG(var, min, max) REGISTER_VAR_SIMPLE(CFG, #var, self->var, min, max)

    #ifdef USE_BT
      REGISTER_CMD(CFG,        "save",      {AllObj.save(false);} )             // save not default
      REGISTER_CMD(CFG,        "load",      {AllObj.load(false);} )             // load not default
      REGISTER_CMD(CFG,        "default",   {AllObj.load(true);}  )             // load default
      REGISTER_CMD_NOSHOW(CFG, "getInits",  {AllObj.sendInits(BT);} )           // answer with all vars init (min, max, value)
      REGISTER_CMD_NOSHOW(CFG, "getUpdate", {AllObj.sendUpdate(BT, Motion);} )  // answer with all updates
    #endif

    REGISTER_CFG(stripMid,   0, 1);
    REGISTER_CFG(stripRear,  0, 1);
    REGISTER_CFG(stripFront, 0, 1);

    REGISTER_CFG(probe,      0, 1);
    REGISTER_CFG(minProbe,   1, MaxProbe);
    REGISTER_CFG(bright,     1, 255);

    REGISTER_CFG(pacifica,   0, 255);
    REGISTER_CFG(fire,       0, 255);

    REGISTER_CFG(runSpeed,   0, 10);
    REGISTER_CFG(neutralWZ,  0, 32768);
    REGISTER_CFG(maxWZ,      0, 32768);

    REGISTER_CFG(divAcc,     1, 10);
    REGISTER_CFG(smoothAcc,  1, 32768);
    REGISTER_CFG(thresAcc,   0, 255);

    REGISTER_CFG(minEye,     1, (NBLEDS_TIPS>>1));
    REGISTER_CFG(maxEye,     1, (NBLEDS_TIPS>>1));

    REGISTER_CFG(minDim,     1, 10);
    REGISTER_CFG(maxDim,     1, 10);
    REGISTER_CFG(minTwkR,    0, 255);
  };
};
