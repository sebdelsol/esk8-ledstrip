
#include <bluetooth.h>
#include <stdarg.h>
#include <rom/rtc.h>

BluetoothSerial BTSerial;
BTcmd BTcmd(BTSerial);

bool Connected = false;

void BTcallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param){
  if(event == ESP_SPP_SRV_OPEN_EVT){
    Serial << "Client Connected" << endl;
    Connected = true;
  }
  else if(event == ESP_SPP_CLOSE_EVT){
    Serial << "Client DisConnected" << endl;
    Connected = false;
  }
}

// inits
BlueTooth::BlueTooth()
{
  mBTserial = &BTSerial;
  mBTcmd = &BTcmd;
}

static __NOINIT_ATTR bool WasOn; // HACK

void BlueTooth::init(bool on)
{
  if (rtc_get_reset_reason(0) == 12) on = WasOn; // HACK, SW reset is proly crash
  // Serial << "reset " << rtc_get_reset_reason(0) << endl;

  Serial << "Init BT" << endl;
  BTSerial.register_callback(BTcallback);
  pinMode(LIGHT_PIN, OUTPUT);
  start(on);
}

// Power management
void BlueTooth::start(const bool on)
{
  WasOn = on; //HACK
  Serial << (on ? "Start" : "Stop") << " BT" << endl;
  if (on){
    mON = BTSerial.begin(BT_TERMINAL_NAME);
  }
  else{
    mON = false;
    BTSerial.end();
  }
  Serial << (mON ? "Started" : "Stopped") << " BT" << endl;
  digitalWrite(LIGHT_PIN, mON ? HIGH : LOW);
  Connected = false;

  if (mON)
    mStartTime = millis();
}

void BlueTooth::toggle()
{
  start(mON ? false : true);
}

bool BlueTooth::update()
{
  if (mON) {
    if (Connected) 
      mBTcmd->readStream();
    else if(millis() - mStartTime > AUTO_STOP_IF_NOTCONNECTED)
      start(false);
  }
  return mON;
}
