#include <OTA.h>

//--------------------------------------
void OTA::begin()
{
  if(WiFi.status() == WL_CONNECTED)
  {
    // OTA_NAME & OTA_PORT are shared by OTA and the socket server, check platformio build_flags
    _log << "OTA started, answer @ " << OTA_NAME << ".local:" << OTA_PORT << endl;
    ArduinoOTA.setHostname(OTA_NAME).setPort(OTA_PORT).setMdnsEnabled(!MDNS.begin(OTA_NAME)); 
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
