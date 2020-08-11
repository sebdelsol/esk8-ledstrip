#include <Bluetooth.h>

//------------------------------------------------------------
BlueTooth* CurrentBT;

void CallbackWrapper(esp_spp_cb_event_t event, esp_spp_cb_param_t* param)
{
  CurrentBT->onEvent(event, param);
}

void BlueTooth::onEvent(esp_spp_cb_event_t event, esp_spp_cb_param_t* param)
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

//------------------------------------------------------------
void BlueTooth::init(const bool on)
{
  CurrentBT = this;
  mBTSerial.register_callback(CallbackWrapper);
  start(on);

  pinMode(LIGHT_PIN, OUTPUT); //blue led
}

//------------------------------------------------------------
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

    mDbgSerial << "BT has " << (mON ? "Started" : "Stopped") << endl;
    digitalWrite(LIGHT_PIN, mON ? HIGH : LOW);
  }
}

void BlueTooth::toggle()
{
  start(mON ? false : true);
}

//------------------------------------------------------------
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
