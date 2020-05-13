#pragma once

#include <Streaming.h>
#include <ArduinoOTA.h>   

#define OTA_HOSTNAME "esk8" // see platformio.ini where >> upload_port = "esk8.local"

class OTA
{
    Stream& mSerial;

    bool mBegun = false;
    void begin();

public:
    OTA(Stream& dbgSerial);
    void update();
};
