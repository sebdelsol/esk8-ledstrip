# eSK8 LED strips
***LED strips controlled by an accelerometer for an eSK8 with a basic companion app***

<img src="https://media.giphy.com/media/IhCHKo42Hx7WFkRmzQ/giphy.gif" height="300"> <img src="https://media.giphy.com/media/fY5xLxGayUptPZuTfG/giphy.gif" height="300"> <img src="https://media.giphy.com/media/RfYtkG17dUJyVmbPet/giphy.gif" height="300">

# Ckick to load [PCB & Schematic](https://easyeda.com/seb.morin/esk8) 

[<img src="https://image.easyeda.com/histories/aaf838e4a54c468f9502dc529522ac38.png" height="225">](https://easyeda.com/seb.morin/esk8) [<img src="https://i.imgur.com/bn5Pk2N.jpg" height="225">](https://easyeda.com/seb.morin/esk8) [<img src="https://i.imgur.com/fsrZ5Zs.jpg" height="225">](https://easyeda.com/seb.morin/esk8)

# Android App 
<img src="https://i.imgur.com/sUIXf7x.jpg" height="300" align="right">

If you want to tweak values in real time :
* login on **[app inventor](http://ai2.appinventor.mit.edu/)** & import [esk8.aia](https://github.com/sebdelsol/Esk8/blob/master/esk8.aia)
* pair your android's phone with the **Esk8** device *within 5s after power on*
* use *REGISTER_VAR* & *REGISTER_CMD* macros to automatically create app's widgets
<p>&nbsp;</p>  <p>&nbsp;</p>  <p>&nbsp;</p>  <p>&nbsp;</p>  

# LED strips debug
<img src="https://media.giphy.com/media/eJFgXPfn9yUhgEfCkM/giphy.gif" height="300" align="right">

If you want to check what's happening with your LED strips : 
* in [main.cpp](https://github.com/sebdelsol/Esk8/blob/master/src/main.cpp) uncomment `#define DEBUG_LED_TOWIFI`
* please install [PyGame](https://www.pygame.org) & [SimpleWebsocketServer](https://pypi.org/project/simple-websocket-server) then launch [debugLedstrip.py](https://github.com/sebdelsol/Esk8/blob/master/DebugLedstrip.py)

# OTA update & Telnet debug
If you want **OTA** update : 
* in [main.cpp](https://github.com/sebdelsol/Esk8/blob/master/src/main.cpp) uncomment `#define USE_OTA`
* in [platformio.ini](https://github.com/sebdelsol/Esk8/blob/master/platformio.ini) uncomment `upload_protocol = espota` & `upload_port = "esk8.local"`

If you want **Telnet** logs when using OTA : 
* in [main.cpp](https://github.com/sebdelsol/Esk8/blob/master/src/main.cpp) uncomment `#define USE_TELNET`
* launch a cmd line & type `telnet`, then `open esk8.local`

# BOM

Qty | Components | *20€*
---:| :---| ---:
1| PCB| *2€*
1|[Lolin32](https://wiki.wemos.cc/products:lolin32:lolin32)| *4€*
1|[SN74AHCT125](https://www.ti.com/product/SN74AHCT125) | *50c*
1|[MPU 6050](https://invensense.tdk.com/products/motion-tracking/6-axis/mpu-6050/) | *1€*
50cm|WS2812B IP67, 144Leds/m | *8€*
1m|WS2812B IP67, 60Leds/m | *4€*
3|470Ω resistors | *1c*
1|1000μF capacitor | *1c*
1|10kΩ resistor | *1c*
1|LDR | *1c*
1|220Ω resistor | *1c*
1|blue LED | *1c*
1|12x12 switch | *1c*
