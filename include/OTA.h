#pragma once

#define NODEBUG_WEBSOCKETS
#define OTA_HOSTNAME "esk8" // see platformio.ini where >> upload_port = "esk8.local"

class OTA
{
    bool mOn = false;
public:
    void begin();
    void update();
};
