#pragma once

#define NODEBUG_WEBSOCKETS

class OTA
{
long startTime;
bool started;

public:
    void begin();
    void setup()    ;
    void update();
};
