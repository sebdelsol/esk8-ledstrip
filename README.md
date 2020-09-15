# show
<img src="https://media.giphy.com/media/IhCHKo42Hx7WFkRmzQ/giphy.gif" height="225" align="right">
<img src="https://media.giphy.com/media/fY5xLxGayUptPZuTfG/giphy.gif" height="225" align="right">
<img src="https://media.giphy.com/media/RfYtkG17dUJyVmbPet/giphy.gif" height="225" align="right">

**Leds controlled by movement & a [phone app](https://github.com/sebdelsol/esk8-ledstrip/blob/master/README.md#Customize)**
<p>&nbsp;</p> <p>&nbsp;</p> <p>&nbsp;</p> <p>&nbsp;</p>

# build
[<img src="https://i.imgur.com/fsrZ5Zs.jpg" height="185" align="right">](https://easyeda.com/seb.morin/esk8) 
[<img src="https://i.imgur.com/bn5Pk2N.jpg" height="185" align="right">](https://easyeda.com/seb.morin/esk8)
[<img src="https://image.easyeda.com/histories/aaf838e4a54c468f9502dc529522ac38.png" height="185" align="right">](https://easyeda.com/seb.morin/esk8)

**check [PCB & schematic](https://easyeda.com/seb.morin/esk8)**
<p>&nbsp;</p> <p>&nbsp;</p> <p>&nbsp;</p>

# customize
<img src="https://media.giphy.com/media/TfFm0aNsc1LnWPsiab/giphy.gif" height="250" align="right">

**tweak in real time on a phone**
* use easy macros to **create new app's widgets**, check [objVar.h](https://github.com/sebdelsol/esk8-ledstrip/blob/cfee2ca77b1fcf0d9df5d271e498db0bf86edbf7/include/objVar.h#L96)
* import [esk8.aia](https://github.com/sebdelsol/esk8-ledstrip/blob/master/Esk8.aia) on [App Inventor](http://ai2.appinventor.mit.edu/)
* pair your phone with the `esk8` bluetooth device *within 30s after being switched on*
<p>&nbsp;</p>  <p>&nbsp;</p>  

# iterate
<img src="https://media.giphy.com/media/eJFgXPfn9yUhgEfCkM/giphy.gif" height="215" align="right">

**check the Ledstrips on PC**
* uncomment `#define USE_LEDSERVER` in [main.cpp](https://github.com/sebdelsol/esk8-ledstrip/blob/870b272afb6b136938d0b67caa385b4bf29b96c0/src/main.cpp#L4) 
* launch [debugLedstrip.py](https://github.com/sebdelsol/esk8-ledstrip/blob/master/debugLedstrip.py) - dependency : [PyGame](https://www.pygame.org)

<p>&nbsp;</p>  <p>&nbsp;</p> 

# over the air
* check [myWifi.h](https://github.com/sebdelsol/esk8-ledstrip/blob/cfee2ca77b1fcf0d9df5d271e498db0bf86edbf7/include/myWifi.h#L8) how to create `Wificonfig.h` 

**upload**
* uncomment `#define USE_OTA` in [main.cpp](https://github.com/sebdelsol/esk8-ledstrip/blob/870b272afb6b136938d0b67caa385b4bf29b96c0/src/main.cpp#L2) 
* uncomment`upload_*` defines in [platformio.ini](https://github.com/sebdelsol/esk8-ledstrip/blob/d1ec9845f7aaaaa8174fc5b309df723bf2c1d1d1/platformio.ini#L25-27)

**log**
* uncomment `#define USE_TELNET` in [main.cpp](https://github.com/sebdelsol/esk8-ledstrip/blob/870b272afb6b136938d0b67caa385b4bf29b96c0/src/main.cpp#L3) 
* launch [telnet.py](https://github.com/sebdelsol/esk8-ledstrip/blob/master/telnet.py)

# shop
*Components* | *20€* | *Shop*
:---| ---: | :---:
<sub>custom **[PCB](https://easyeda.com/seb.morin/esk8)**| <sub>2€</sub>| [ⓘ](https://easyeda.com/seb.morin/esk8)</sub>
<sub> Wemos **[Lolin32](https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32_datasheet_en.pdf)**</sub>| <sub>4€</sub> | [ⓘ](https://www.aliexpress.com/wholesale?catId=0&SearchText=lolin32)
<sub> TI **[SN74AHCT125](https://www.ti.com/product/SN74AHCT125)** </sub>| <sub>50c</sub> | [ⓘ](https://www.ebay.com/sch/i.html?_nkw=SN74AHCT125)
<sub> InvenSense **[MPU-6050](https://invensense.tdk.com/products/motion-tracking/6-axis/mpu-6050/)** </sub>| <sub>1€</sub> | [ⓘ](https://www.aliexpress.com/wholesale?catId=0&SearchText=mpu-6050)
<sub>1.5m IP65 **[WS2812B](https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf)** w/144 leds/m </sub>| <sub>12€</sub> | [ⓘ](https://www.aliexpress.com/wholesale?catId=0&SearchText=ws2812b+ip67)
<sub>3x *470Ω R*, *1000μF* C, *10kΩ* R, LDR, *220Ω* R, *blue* LED, *12x12* switch</sub>| <sub>50c</sub> | .

# repear
* to compile I2CDevLib add `#define BUFFER_LENGTH I2C_BUFFER_LENGTH` in `I2Cdev.h`
* check **sloc** [here](https://api.codetabs.com/v1/loc/?github=sebdelsol/esk8-ledstrip)
