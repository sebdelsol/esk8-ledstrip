# eSK8 LEDstrips
<img src="https://media.giphy.com/media/IhCHKo42Hx7WFkRmzQ/giphy.gif" height="215" align="right"><img src="https://media.giphy.com/media/fY5xLxGayUptPZuTfG/giphy.gif" height="215" align="right"><img src="https://media.giphy.com/media/RfYtkG17dUJyVmbPet/giphy.gif" height="215" align="right">

***LEDstrips* controlled by an *accelerometer* for an *eSK8* with a phone *companion app***
<p>&nbsp;</p>  <p>&nbsp;</p>   <p>&nbsp;</p>  

# [PCB & Schematic](https://easyeda.com/seb.morin/esk8) 
[<img src="https://image.easyeda.com/histories/aaf838e4a54c468f9502dc529522ac38.png" height="185" align="right">](https://easyeda.com/seb.morin/esk8)[<img src="https://i.imgur.com/bn5Pk2N.jpg" height="185" align="right">](https://easyeda.com/seb.morin/esk8)[<img src="https://i.imgur.com/fsrZ5Zs.jpg" height="185" align="right">](https://easyeda.com/seb.morin/esk8)

**click to check the EasyEDA project**

<p>&nbsp;</p>  <p>&nbsp;</p>   <p>&nbsp;</p>  

# Android App 
<img src="https://media.giphy.com/media/dtBonRsITrgrtEBi5O/giphy.gif" height="250" align="right">

**if you want to tweak values in *real time*** on your phone
* login on **[app inventor](http://ai2.appinventor.mit.edu/)** & import **[esk8.aia](https://github.com/sebdelsol/Esk8/blob/master/esk8.aia)**
* pair your Android phone with the **Esk8** device *within 5s after power on*
* use `REGISTER_VAR` & `REGISTER_CMD` macros to automatically create app's widgets
<p>&nbsp;</p>  <p>&nbsp;</p>  

# LEDstrips debug
<img src="https://media.giphy.com/media/eJFgXPfn9yUhgEfCkM/giphy.gif" height="215" align="right">

**if you want to check what's happening with your *LEDstrips*** on your computer
* uncomment `#define DEBUG_LED_TOWIFI` in **[main.cpp](https://github.com/sebdelsol/Esk8/blob/master/src/main.cpp)** 
* please install **[PyGame](https://www.pygame.org)** & **[SimpleWebsocketServer](https://pypi.org/project/simple-websocket-server)** then launch **[debugLedstrip.py](https://github.com/sebdelsol/Esk8/blob/master/DebugLedstrip.py)**

<p>&nbsp;</p>  <p>&nbsp;</p>

# OTA update & Telnet log
**if you want *OTA* update**
* uncomment `#define USE_OTA` in **[main.cpp](https://github.com/sebdelsol/Esk8/blob/master/src/main.cpp)** 
* uncomment `upload_protocol = espota` & `upload_port = "esk8.local"` in **[platformio.ini](https://github.com/sebdelsol/Esk8/blob/master/platformio.ini)** 

**if you want *Telnet* logs when using OTA** on your computer
* uncomment `#define USE_TELNET` in **[main.cpp](https://github.com/sebdelsol/Esk8/blob/master/src/main.cpp)** 
* launch a  `telnet` cmd line & `open esk8.local`

# BOM

Qty | Components | *20€*
---:| :---| ---:
<sub>1</sub>|**[custom PCB](https://easyeda.com/seb.morin/esk8)**| *2€*
<sub>1</sub>|**[Lolin32](https://wiki.wemos.cc/products:lolin32:lolin32)**| *4€*
<sub>1</sub>|**[SN74AHCT125](https://www.ti.com/product/SN74AHCT125)** | *50c*
<sub>1</sub>|**[MPU-6050](https://invensense.tdk.com/products/motion-tracking/6-axis/mpu-6050/)** | *1€*
<sub>0.5m</sub>|**[WS2812B](https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf)** with **[IP67](https://en.wikipedia.org/wiki/IP_Code)**, 144 leds/m | *8€*
<sub>1m</sub>|**[WS2812B](https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf)** with **[IP67](https://en.wikipedia.org/wiki/IP_Code)**, 60 leds/m | *4€*
<sub>3</sub>|<sub>470Ω resistors</sub> | -
<sub>1</sub>|<sub>1000μF capacitor</sub> | -
<sub>1</sub>|<sub>10kΩ resistor</sub> | -
<sub>1</sub>|<sub>LDR</sub> | -
<sub>1</sub>|<sub>220Ω resistor</sub> | -
<sub>1</sub>|<sub>blue LED</sub> | -
<sub>1</sub>|<sub>12x12 switch</sub> | -

# Dependencies

* in **BluetoothSerial.cpp** modify `#define TX_QUEUE_SIZE 512` to avoid bluetooth congestion
* in **MPU6050_6Axis_MotionApps20.h** you'll need to comment those typedef `prog_int8_t`, `prog_int32_t` & `prog_uint32_t`
* get TelnetSpy from **[github](https://github.com/yasheena/telnetspy/)** instead of PlatformIO
