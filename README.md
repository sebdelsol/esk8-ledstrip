# eSK8 LEDstrips
<img src="https://media.giphy.com/media/IhCHKo42Hx7WFkRmzQ/giphy.gif" height="225" align="right"><img src="https://media.giphy.com/media/fY5xLxGayUptPZuTfG/giphy.gif" height="225" align="right"><img src="https://media.giphy.com/media/RfYtkG17dUJyVmbPet/giphy.gif" height="225" align="right">

**controlled by an *accelerometer* with a *[companion app](https://github.com/sebdelsol/esk8-ledstrip/blob/master/README.md#android-app)***
<p>&nbsp;</p>  <p>&nbsp;</p>   <p>&nbsp;</p>  

# PCB & Schematic
[<img src="https://image.easyeda.com/histories/aaf838e4a54c468f9502dc529522ac38.png" height="185" align="right">](https://easyeda.com/seb.morin/esk8)[<img src="https://i.imgur.com/bn5Pk2N.jpg" height="185" align="right">](https://easyeda.com/seb.morin/esk8)[<img src="https://i.imgur.com/fsrZ5Zs.jpg" height="185" align="right">](https://easyeda.com/seb.morin/esk8)

**check the [EasyEDA project](https://easyeda.com/seb.morin/esk8)**

<p>&nbsp;</p>  <p>&nbsp;</p>   <p>&nbsp;</p>  

# Android App 
<img src="https://media.giphy.com/media/TfFm0aNsc1LnWPsiab/giphy.gif" height="250" align="right">

**if you want to *tweak* your setup in real time on your *phone***
* login on **[App Inventor](http://ai2.appinventor.mit.edu/)** & import **[esk8.aia](https://github.com/sebdelsol/Esk8/blob/master/esk8.aia)**
* pair your Android phone with the **Esk8** device *within 5s after power on*
* use `REGISTER_VAR` & `REGISTER_CMD` macros to automatically create app's widgets
<p>&nbsp;</p>  <p>&nbsp;</p>  

# LEDstrips debug
<img src="https://media.giphy.com/media/eJFgXPfn9yUhgEfCkM/giphy.gif" height="215" align="right">

**if you want to *check* what's happening with your *LEDstrips***
* uncomment `#define DEBUG_LED_TOWIFI` in **[main.cpp](https://github.com/sebdelsol/Esk8/blob/master/src/main.cpp)** 
* please install **[PyGame](https://www.pygame.org)** & **[SimpleWebsocketServer](https://pypi.org/project/simple-websocket-server)** then launch **[debugLedstrip.py](https://github.com/sebdelsol/Esk8/blob/master/DebugLedstrip.py)**

<p>&nbsp;</p>  <p>&nbsp;</p>

# OTA update & Telnet log
**if you want *OTA* update**
* uncomment `#define USE_OTA` in **[main.cpp](https://github.com/sebdelsol/Esk8/blob/master/src/main.cpp)** 
* uncomment `upload_protocol = espota` & `upload_port = "esk8.local"` in **[platformio.ini](https://github.com/sebdelsol/Esk8/blob/master/platformio.ini)** 

**if you want *Telnet* logs**
* uncomment `#define USE_TELNET` in **[main.cpp](https://github.com/sebdelsol/Esk8/blob/master/src/main.cpp)** 
* launch a  `telnet` cmd line & `open esk8.local`

# BOM

*Qty* | *Components* | *20€* | *Shop*
---:| :---| ---: | :---:
<sub>1x</sub>| <sub>Custom **[PCB](https://easyeda.com/seb.morin/esk8)**| <sub>2€</sub>| [ⓘ](https://easyeda.com/seb.morin/esk8)</sub>
<sub>1x</sub>|<sub> Wemos **[Lolin32](https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32_datasheet_en.pdf)**</sub>| <sub>4€</sub> | [ⓘ](https://www.aliexpress.com/wholesale?catId=0&SearchText=lolin32)
<sub>1x</sub>|<sub> Texas Instruments **[SN74AHCT125](https://www.ti.com/product/SN74AHCT125)** </sub>| <sub>50c</sub> | [ⓘ](https://www.ebay.com/sch/i.html?_nkw=SN74AHCT125)
<sub>1x</sub>|<sub> InvenSense **[MPU-6050](https://invensense.tdk.com/products/motion-tracking/6-axis/mpu-6050/)** </sub>| <sub>1€</sub> | [ⓘ](https://www.aliexpress.com/wholesale?catId=0&SearchText=mpu-6050)
<sub>1m</sub>|<sub>**[IP67](https://en.wikipedia.org/wiki/IP_Code)** Neopixel **[WS2812B](https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf)** 60 leds/m </sub>| <sub>5€</sub> | [ⓘ](https://www.aliexpress.com/wholesale?catId=0&SearchText=ws2812b+ip67)
<sub>0.5m</sub>|<sub>**[IP67](https://en.wikipedia.org/wiki/IP_Code)** Neopixel **[WS2812B](https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf)** 144 leds/m </sub>| <sub>7€</sub> | [ⓘ](https://www.aliexpress.com/wholesale?catId=0&SearchText=ws2812b+ip67)
<sub>3x</sub>|<sub>470Ω resistors</sub> | . | .
<sub>1x</sub>|<sub>1000μF capacitor</sub> | . | .
<sub>1x</sub>|<sub>10kΩ resistor</sub> | . | .
<sub>1x</sub>|<sub>LDR</sub> | . | .
<sub>1x</sub>|<sub>220Ω resistor</sub> | . | .
<sub>1x</sub>|<sub>Blue LED</sub> | . | .
<sub>1x</sub>|<sub>12x12 switch</sub> | . | .

# Dependencies

* in **BluetoothSerial.cpp** modify `#define TX_QUEUE_SIZE 128` to avoid bluetooth congestion
* in **MPU6050_6Axis_MotionApps20.h** you'll need to comment those typedef `prog_int8_t`, `prog_int32_t` & `prog_uint32_t`
* get TelnetSpy from **[github](https://github.com/yasheena/telnetspy/)** instead of PlatformIO
