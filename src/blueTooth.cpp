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
    _log << "BT client Connected @ ";
    for (int i = 0; i < 6; i++)
      _log << _HEX(param->srv_open.rem_bda[i]) << (i < 5 ? ":" : "");
    _log << endl;

    mConnected = true;
  }
  else if(event == ESP_SPP_CLOSE_EVT)
  {
    _log << "BT client DisConnected" << endl;
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
    _log << "BT " << (on ? "Starting" : "Stopping") << "...";
    mConnected = false;
  
    digitalWrite(LIGHT_PIN, on ? HIGH : LOW); // faster feedbcack might be false
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
    digitalWrite(LIGHT_PIN, mON ? HIGH : LOW); // actual feedback

    _log << "BT " << (mON ? "Started" : "Stopped") << endl;
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
