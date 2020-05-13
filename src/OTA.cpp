#include <OTA.h>

OTA::OTA(Stream& serial) : mSerial(serial) {}  

void OTA::begin()
{
  if(WiFi.status() == WL_CONNECTED)
  {
    mSerial << "OTA Connected @ " << WiFi.localIP() << endl;
    ArduinoOTA.setHostname(OTA_HOSTNAME);

    ArduinoOTA
      .onStart( [this]()
      {
        mSerial << "Start updating " << (ArduinoOTA.getCommand() == U_FLASH ? "sketch" : "filesystem") << endl;
      })
      
      .onEnd( [this]()
      {
        mSerial << "\nEnd\n";
      })
      
      .onProgress( [this](unsigned int progress, unsigned int total)
      {
        mSerial << "Progress: " << (progress / (total / 100)) << "%\r";
      })
      
      .onError( [this](ota_error_t error)
      {
        mSerial << "Error " << error << endl; // check ArduinoOTA.h for errors
      });

    ArduinoOTA.begin();
    mBegun = true;
  }
}

void OTA::update() 
{
  if (!mBegun)
    begin();
  else
    ArduinoOTA.handle();
}
