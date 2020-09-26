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
  pinMode(BLUE_PIN, OUTPUT); //blue led
  mBTSerial.register_callback(onEvent);
  start(on);
}

//------------------------------------------------------------
void BlueTooth::start(const bool on)
{
  if (mON != on)
  {
    mConnected = false;

    _log << "BT " << (on ? "starting" : "stopping") << "...";
    digitalWrite(BLUE_PIN, on ? HIGH : LOW); // faster feedbcack might be false

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
    
    digitalWrite(BLUE_PIN, mON ? HIGH : LOW); // actual feedback
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
  //handle timeout when not connected
  if (mON && !mConnected && millis() - mStartTime > BT_TIMEOUT) start(false);
  
  return mON && mConnected;
}