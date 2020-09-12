#include <OTA.h>

//--------------------------------------
void OTA::begin()
{
  if(WiFi.status() == WL_CONNECTED)
  {
    _log << "OTA started" << endl;
    ArduinoOTA.setHostname(OTA_HOSTNAME);
    ArduinoOTA.begin();
    mBegun = true;
  }
}

//--------------------------------------
void OTA::update() 
{
  if (!mBegun)
    begin();
  else
    ArduinoOTA.handle();
}
