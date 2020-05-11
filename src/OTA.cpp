#include <OTA.h>

void OTA::begin()
{
  if(WiFi.status() == WL_CONNECTED)
  {
    Serial << "OTA Connected @ " << WiFi.localIP() << endl;
    ArduinoOTA.setHostname(OTA_HOSTNAME);

    ArduinoOTA
      .onStart([]() {
        Serial << "Start updating " << (ArduinoOTA.getCommand() == U_FLASH ? "sketch" : "filesystem") << endl;
      })
      .onEnd([]() {
        Serial << "\nEnd\n";
      })
      .onProgress([](unsigned int progress, unsigned int total) {
        Serial << "Progress: " << (progress / (total / 100)) << "%\r";
      })
      .onError([](ota_error_t error) {
        Serial << "Error " << error << endl; // check ArduinoOTA.h for errors
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
