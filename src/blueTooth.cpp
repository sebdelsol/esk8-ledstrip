
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
  pinMode(GREEN_PIN, OUTPUT);
  start(on);
}

// Power management
void BlueTooth::start(const bool on)
{
  WasOn = on; //HACK
  Serial << (on ? "Start" : "Stop") << " BT" << endl;
  digitalWrite(GREEN_PIN, on ? HIGH : LOW);
  if (on){
    mON = BTSerial.begin(BT_TERMINAL_NAME);
    digitalWrite(GREEN_PIN, mON ? HIGH : LOW);
  }
  else{
    mON = false;
    BTSerial.end();
  }
  Connected = false;
  // Serial << (mON ? "Start" : "Stop") << " BT done" << endl;
}

void BlueTooth::toggle()
{
  start(mON ? false : true);
}

bool BlueTooth::update()
{
  if (mON & Connected) mBTcmd->readStream();
  return mON;
}
