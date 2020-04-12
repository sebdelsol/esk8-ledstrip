# eSK8 LEDstrips
<img src="https://media.giphy.com/media/IhCHKo42Hx7WFkRmzQ/giphy.gif" height="215" align="right"><img src="https://media.giphy.com/media/fY5xLxGayUptPZuTfG/giphy.gif" height="215" align="right"><img src="https://media.giphy.com/media/RfYtkG17dUJyVmbPet/giphy.gif" height="215" align="right">

**LED strips controlled by an accelerometer for an eSK8 with a phone companion app**
<p>&nbsp;</p>  <p>&nbsp;</p>   <p>&nbsp;</p>  

# [PCB & Schematic](https://easyeda.com/seb.morin/esk8) 
[<img src="https://image.easyeda.com/histories/aaf838e4a54c468f9502dc529522ac38.png" height="185" align="right">](https://easyeda.com/seb.morin/esk8)[<img src="https://i.imgur.com/bn5Pk2N.jpg" height="185" align="right">](https://easyeda.com/seb.morin/esk8)[<img src="https://i.imgur.com/fsrZ5Zs.jpg" height="185" align="right">](https://easyeda.com/seb.morin/esk8)

**click to check the EasyEDA project**

<p>&nbsp;</p>  <p>&nbsp;</p>   <p>&nbsp;</p>  

# Android App 
<img src="https://i.imgur.com/sUIXf7x.jpg" height="250" align="right">

**If you want to tweak values in real time**
* login on [app inventor](http://ai2.appinventor.mit.edu/) & import [esk8.aia](https://github.com/sebdelsol/Esk8/blob/master/esk8.aia)
* pair your android's phone with the **Esk8** device *within 5s after power on*
* use `REGISTER_VAR` & `REGISTER_CMD` macros to automatically create app's widgets
<p>&nbsp;</p>  <p>&nbsp;</p>  

# LED strips debug
<img src="https://media.giphy.com/media/eJFgXPfn9yUhgEfCkM/giphy.gif" height="215" align="right">

**If you want to check what's happening with your LEDstrips**
* uncomment `#define DEBUG_LED_TOWIFI` in [main.cpp](https://github.com/sebdelsol/Esk8/blob/master/src/main.cpp) 
* please install [PyGame](https://www.pygame.org) & [SimpleWebsocketServer](https://pypi.org/project/simple-websocket-server) then launch [debugLedstrip.py](https://github.com/sebdelsol/Esk8/blob/master/DebugLedstrip.py)

<p>&nbsp;</p>  <p>&nbsp;</p>

# OTA update & Telnet log
**If you want *OTA* update**
* uncomment `#define USE_OTA` in [main.cpp](https://github.com/sebdelsol/Esk8/blob/master/src/main.cpp) 
* uncomment `upload_protocol = espota` & `upload_port = "esk8.local"` in [platformio.ini](https://github.com/sebdelsol/Esk8/blob/master/platformio.ini) 

**If you want *Telnet* logs when using OTA**
* uncomment `#define USE_TELNET` in [main.cpp](https://github.com/sebdelsol/Esk8/blob/master/src/main.cpp) 
* launch a  `telnet` cmd line & `open esk8.local`

# BOM

Qty | Components | *20€*
---:| :---| ---:
1|[PCB](https://easyeda.com/seb.morin/esk8)| *2€*
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
