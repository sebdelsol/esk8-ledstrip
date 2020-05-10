#pragma once

#ifdef USE_BT
  #include  <bluetooth.h>
#endif

// ----------------------------------------------------
#define   SERIAL_BAUD   115200  // ms
#define   LED_MAX_MA    800     // mA, please check Cfg.bright to avoid reaching this value

#define   LED_TICK      10      // ms, it's used too  for mpu6050 which is refreshed every 10ms
#define   BT_TICK       30      // ms

#define   NBLEDS_MIDDLE 30
#define   NBLEDS_TIPS   36

// ----------------------------------------------------
class CFG : public OBJVar
{
public:
  // update ?
  bool ledR       = true;
  bool ledF       = true;
  bool led        = true;

  // brightness ?
  byte bright     = 128;  // half brightness is enough to avoid reaching LED_MAX_MA
  int fade        = 0;    // for the fade in
  
  #define MaxProbe 4095
  int  minProbe   = 400;
  bool probe      = false;

  byte pacifica   = 255;
  byte fire       = 0;

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
  int minTwkR     = 128;

  #ifdef USE_BT
    BlueTooth &BT;
    CFG(BlueTooth& BT) : BT(BT) {};
  #endif

  void init()
  {
    #define REGISTER_CFG(var, min, max) REGISTER_VAR_SIMPLE(CFG, #var, self->var, min, max)

    #ifdef USE_BT
      REGISTER_CMD(CFG,        "save",      {self->BT.save(false);} )       // save not default
      REGISTER_CMD(CFG,        "load",      {self->BT.load(false);} )       // load not default
      REGISTER_CMD(CFG,        "default",   {self->BT.load(true);}  )       // load default
      REGISTER_CMD_NOSHOW(CFG, "getInits",  {self->BT.sendInitsOverBT();} ) // answer with all vars init (min, max, value)
      REGISTER_CMD_NOSHOW(CFG, "getUpdate", {self->BT.sendUpdate();} )      // answer with all updates
    #endif

    REGISTER_CFG(ledR,       0, 1);
    REGISTER_CFG(ledF,       0, 1);
    REGISTER_CFG(led,        0, 1);

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
