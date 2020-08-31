#include <OTA.h>

//--------------------------------------
void OTA::begin()
{
  if(WiFi.status() == WL_CONNECTED)
  {
    _log << "OTA  Connected @ " << WiFi.localIP() << endl;
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
