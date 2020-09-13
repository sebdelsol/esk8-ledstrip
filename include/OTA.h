#pragma once

#include <ArduinoOTA.h>   
#include <ESPmDNS.h>
#include <log.h>

class OTA
{
    bool mBegun = false;
    void begin();

public:
    void update();
};
