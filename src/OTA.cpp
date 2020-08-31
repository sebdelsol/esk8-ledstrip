#include <OTA.h>

//--------------------------------------
void OTA::begin()
{
  if(WiFi.status() == WL_CONNECTED)
  {
    _log << "OTA  Connected @ " << WiFi.localIP() << endl;
    ArduinoOTA.setHostname(OTA_HOSTNAME);

    ArduinoOTA
      .onStart( [this]()
      {
        _log << "Start updating " << (ArduinoOTA.getCommand() == U_FLASH ? "sketch" : "filesystem") << endl;
      })
      
      .onEnd( [this]()
      {
        _log << "\nEnd\n";
      })
      
      .onProgress( [this](unsigned int progress, unsigned int total)
      {
        _log << "Progress: " << (progress / (total / 100)) << "%\r";
      })
      
      .onError( [this](ota_error_t error)
      {
        _log << "Error " << error << endl; // check ArduinoOTA.h for errors
      });

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
