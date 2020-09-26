#include <Bluetooth.h>

// needed in static onEvent
bool BlueTooth::mConnected = false;
long BlueTooth::mStartTime;

//------------------------------------------------------------
void BlueTooth::onEvent(esp_spp_cb_event_t event, esp_spp_cb_param_t* param)
{
  if(event == ESP_SPP_SRV_OPEN_EVT)
  {
    _log << "BT client connected @ ";
    for (int i = 0; i < 6; i++) _log << _HEX(param->srv_open.rem_bda[i]) << ( i < 5 ? ":" : "\n" );

    mConnected = true;
  }
  else if(event == ESP_SPP_CLOSE_EVT)
  {
    _log << "BT client disConnected" << endl;

    mConnected = false;
    mStartTime = millis(); 
  }
}

//------------------------------------------------------------
void BlueTooth::init(const bool on)
{
  pinMode(LIGHT_PIN, OUTPUT); //blue led
  mBTSerial.register_callback(BlueTooth::onEvent);
  start(on);
}

//------------------------------------------------------------
void BlueTooth::start(const bool on)
{
  if (mON != on)
  {
    mConnected = false;

    _log << "BT " << (on ? "starting" : "stopping") << "...";
    digitalWrite(LIGHT_PIN, on ? HIGH : LOW); // faster feedbcack might be false

    if (on)
    {
      mON = mBTSerial.begin(BT_SERVER_NAME);
      if (mON) mStartTime = millis();
    }
    else 
    {
      mON = false;
      mBTSerial.end();
    }
    
    digitalWrite(LIGHT_PIN, mON ? HIGH : LOW); // actual feedback
    _log << "BT " << (mON ? "started" : "stopped") << endl;
  }
}

void BlueTooth::toggle()
{
  start(!mON);
}

//------------------------------------------------------------
bool BlueTooth::isReady()
{
  if (mON)
  {
    if (mConnected)
      return true;
    else if(millis() - mStartTime > BT_TIMEOUT)
      start(false);
  }
  return false;
}