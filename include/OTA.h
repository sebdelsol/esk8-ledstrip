#pragma once

#define NODEBUG_WEBSOCKETS

class OTA
{
    bool mOn = false;
public:
    void begin();
    void update();
};
