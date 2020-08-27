# Show
<img src="https://media.giphy.com/media/IhCHKo42Hx7WFkRmzQ/giphy.gif" height="225" align="right">
<img src="https://media.giphy.com/media/fY5xLxGayUptPZuTfG/giphy.gif" height="225" align="right">
<img src="https://media.giphy.com/media/RfYtkG17dUJyVmbPet/giphy.gif" height="225" align="right">

**Leds controlled by movement & a [phone app](https://github.com/sebdelsol/esk8-ledstrip/blob/master/README.md#Customize)**
<p>&nbsp;</p> <p>&nbsp;</p> <p>&nbsp;</p> <p>&nbsp;</p>

# Build
[<img src="https://i.imgur.com/fsrZ5Zs.jpg" height="185" align="right">](https://easyeda.com/seb.morin/esk8) 
[<img src="https://i.imgur.com/bn5Pk2N.jpg" height="185" align="right">](https://easyeda.com/seb.morin/esk8)
[<img src="https://image.easyeda.com/histories/aaf838e4a54c468f9502dc529522ac38.png" height="185" align="right">](https://easyeda.com/seb.morin/esk8)

**Check [PCB & schematic](https://easyeda.com/seb.morin/esk8)**
<p>&nbsp;</p> <p>&nbsp;</p> <p>&nbsp;</p>

# Customize
<img src="https://media.giphy.com/media/TfFm0aNsc1LnWPsiab/giphy.gif" height="250" align="right">

**Tweak in real time on a phone**
* use easy macros to **create new app's widgets**, check [objVar.h](https://github.com/sebdelsol/esk8-ledstrip/blob/e3f43bdf868395875970e63024d04a82ad06267d/include/objVar.h#L83-93)
* import [esk8.aia](https://github.com/sebdelsol/Esk8/blob/master/Esk8.aia) on [App Inventor](http://ai2.appinventor.mit.edu/)
* pair your phone with the `esk8` bluetooth device *within 30s after being switched on*
<p>&nbsp;</p>  <p>&nbsp;</p>  

# Iterate
<img src="https://media.giphy.com/media/eJFgXPfn9yUhgEfCkM/giphy.gif" height="215" align="right">

**Check Ledstrips on PC**
* uncomment `#define DEBUG_LED_TOWIFI` in [main.cpp](https://github.com/sebdelsol/Esk8/blob/master/src/main.cpp) 
* launch [debugLedstrip.py](https://github.com/sebdelsol/Esk8/blob/master/DebugLedstrip.py) - dependencies : [PyGame](https://www.pygame.org) & [SimpleWebsocketServer](https://pypi.org/project/simple-websocket-server) 

<p>&nbsp;</p>  <p>&nbsp;</p> 

# Debug
**Create wifiConfig**
* check [myWifi.h](https://github.com/sebdelsol/esk8-ledstrip/blob/fed4b2dac2e1fe955d3d43a7b7f73ea413290fed/include/myWifi.h#L11) how to create `Wificonfig.h` in [include](https://github.com/sebdelsol/Esk8/blob/master/include/) dir & 

**Update OTA**
* uncomment `#define USE_OTA` in [main.cpp](https://github.com/sebdelsol/Esk8/blob/master/src/main.cpp)
* uncomment `upload_protocol` & `upload_port` in [platformio.ini](https://github.com/sebdelsol/Esk8/blob/master/platformio.ini)

**Get Telnet logs**
* uncomment `#define USE_TELNET` in [main.cpp](https://github.com/sebdelsol/Esk8/blob/master/src/main.cpp)
* launch a `telnet` session & `open esk8.local`

# Shop
*Components* | *20€* | **ⓘ**
:---| ---: | :---:
<sub>custom **[PCB](https://easyeda.com/seb.morin/esk8)**| <sub>2€</sub>| [ⓘ](https://easyeda.com/seb.morin/esk8)</sub>
<sub> Wemos **[Lolin32](https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32_datasheet_en.pdf)**</sub>| <sub>4€</sub> | [ⓘ](https://www.aliexpress.com/wholesale?catId=0&SearchText=lolin32)
<sub> TI **[SN74AHCT125](https://www.ti.com/product/SN74AHCT125)** </sub>| <sub>50c</sub> | [ⓘ](https://www.ebay.com/sch/i.html?_nkw=SN74AHCT125)
<sub> InvenSense **[MPU-6050](https://invensense.tdk.com/products/motion-tracking/6-axis/mpu-6050/)** </sub>| <sub>1€</sub> | [ⓘ](https://www.aliexpress.com/wholesale?catId=0&SearchText=mpu-6050)
<sub>1.5m IP65 **[WS2812B](https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf)** w/144 leds/m </sub>| <sub>12€</sub> | [ⓘ](https://www.aliexpress.com/wholesale?catId=0&SearchText=ws2812b+ip67)
<sub>3x *470Ω R*, *1000μF* C, *10kΩ* R, LDR, *220Ω* R, *blue* LED, *12x12* switch</sub>| <sub>50c</sub> | .

# Repear
* in `I2Cdev.h` add `#define BUFFER_LENGTH 128` to compile I2CDevLib on an esp32
