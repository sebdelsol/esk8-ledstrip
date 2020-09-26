#include <myWifi.h>

// --------------
void myWifi::stop(const char* reason = nullptr)
{
  WiFi.mode(WIFI_OFF);
  digitalWrite(BUILTIN_LED, HIGH); // led off

  mWantON = false;
  mON = false;
  
  _log << "Wifi stopped..." << ( reason!=nullptr ? reason : "" ) << endl;
}

// --------------
void myWifi::start()
{
  pinMode(BUILTIN_LED, OUTPUT); //internal led
  digitalWrite(BUILTIN_LED, HIGH); // led off

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_ssid, WIFI_password);
  
  mBegunOn = millis();
  mON = false;
  mWantON = true;

  _log << "Wifi started" << endl;
}

// --------------
bool myWifi::update()
{
  if(mWantON && !mON)
  {
    EVERY_N_MILLISECONDS(200)
      digitalWrite(BUILTIN_LED, !digitalRead(BUILTIN_LED)); // blink led

    if(millis() - mBegunOn < WIFI_TIMEOUT)
    {
      if(WiFi.status() == WL_CONNECTED)
      {
        _log << "Wifi connected, answer @ " << WiFi.localIP() << endl;
        digitalWrite(BUILTIN_LED, LOW); // led on
        mON = true;
      }
    }
    else 
      stop("timeout");
  }
  return mON;
}
