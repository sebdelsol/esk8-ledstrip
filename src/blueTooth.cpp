#include <Bluetooth.h>

//------------------------------------------------------------
#include <rom/rtc.h>
static __NOINIT_ATTR bool WasOn; // HACK in case of reboot after a crash

//------------------------------------------------------------
BlueTooth* TrampBT;

void TrampCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t* param)
{
  TrampBT->callback(event, param);
}

//------------------------------------------------------------
void BlueTooth::callback(esp_spp_cb_event_t event, esp_spp_cb_param_t* param)
{
  if(event == ESP_SPP_SRV_OPEN_EVT)
  {
    mDbgSerial << "BT Client Connected @ ";
    for (int i = 0; i < 6; i++)
      mDbgSerial << _HEX(param->srv_open.rem_bda[i]) << (i < 5 ? ":" : "");
    mDbgSerial << endl;

    mConnected = true;
  }
  else if(event == ESP_SPP_CLOSE_EVT)
  {
    mDbgSerial << "BT Client DisConnected" << endl;
    mConnected = false;
  }
}

BlueTooth::BlueTooth(Stream& dbgSerial) : mDbgSerial(dbgSerial) 
{
  mBTbuf.clear();
}

void BlueTooth::init()
{
  TrampBT = this;
  mBTSerial.register_callback(TrampCallback);

  pinMode(LIGHT_PIN, OUTPUT); //blue led

  if (rtc_get_reset_reason(0) == 12 && WasOn) // HACK, SW reset is proly crash, so auto restart BT if it was on
  {
    mDbgSerial << "Reset detected, relaunch BT " << rtc_get_reset_reason(0) << endl;
    start(true);
  }
}

// Power management
void BlueTooth::start(const bool on)
{
  if (mON != on)
  {
    mDbgSerial << "BT " << (on ? "Starting" : "Stopping") << endl;
    mConnected = false;
  
    if (on)
    {
      mON = mBTSerial.begin(BT_TERMINAL_NAME);
      if (mON) mStartTime = millis();
    }
    else 
    {
      mON = false;
      mBTSerial.end();
    }

    WasOn = mON; //HACK
    mDbgSerial << "BT has " << (mON ? "Started" : "Stopped") << endl;
    digitalWrite(LIGHT_PIN, mON ? HIGH : LOW);
  }
}

void BlueTooth::toggle()
{
  start(mON ? false : true);
}

bool BlueTooth::isReadyToReceive()
{
  if (mON)
  {
    if (mConnected)
      return true;
    else if(millis() - mStartTime > AUTO_STOP_IF_NOTCONNECTED)
      start(false);
  }
  return false;
}

bool BlueTooth::isReadyToSend()
{ 
  return mON && mConnected; 
}
