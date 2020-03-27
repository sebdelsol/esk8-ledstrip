# eSK8 motion LEDs
> ***Led strips controlled by an accelerometer for an eSK8 with a basic companion app***

<img src="https://media.giphy.com/media/IhCHKo42Hx7WFkRmzQ/giphy.gif" height="300"> <img src="https://media.giphy.com/media/fY5xLxGayUptPZuTfG/giphy.gif" height="300"> <img src="https://media.giphy.com/media/RfYtkG17dUJyVmbPet/giphy.gif" height="300"> 

# Please check the [PCB & Schematic](https://easyeda.com/seb.morin/esk8) 

[<img src="https://image.easyeda.com/histories/aaf838e4a54c468f9502dc529522ac38.png" height="225">](https://easyeda.com/seb.morin/esk8) [<img src="https://i.imgur.com/bn5Pk2N.jpg" height="225">](https://easyeda.com/seb.morin/esk8) [<img src="https://i.imgur.com/fsrZ5Zs.jpg" height="225">](https://easyeda.com/seb.morin/esk8)

# OTA
If you need OTA update, please uncomment ```#define USE_OTA ``` **USE_OTA** in [main.cpp](https://github.com/sebdelsol/Esk8/blob/master/src/main.cpp)
And uncomment **upload_protocol** & **upload_port** in [platformio.ini](https://github.com/sebdelsol/Esk8/blob/master/platformio.ini)
```Python
upload_protocol = espota
upload_port = "esk8.local"
```

#  Debug LedStrip
If you need to check what's happening with your ledstrip, please uncomment ```#define DEBUG_LED_TOWIFI``` in [main.cpp](https://github.com/sebdelsol/Esk8/blob/master/src/main.cpp)
And launch [DebugLedstrip.py](https://github.com/sebdelsol/Esk8/blob/master/DebugLedstrip.py)

<img src="https://media.giphy.com/media/eJFgXPfn9yUhgEfCkM/giphy.gif" height="300">

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
