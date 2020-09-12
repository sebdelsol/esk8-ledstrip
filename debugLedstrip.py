import pygame
import math
import os
import traceback

from ctypes import windll
SetWindowPos = windll.user32.SetWindowPos
TOPMOST = -1
NOSIZE = 1
NOMOVE = 2

#----------------------------------------------------------------
wPixel = 25
cPixel = wPixel * .25 
minPixel = wPixel * .6
maxPixel = int(round(wPixel * 1.))
REM = 0.5
GAMMA = 5.

#----------------------------------------------------------------
class Pixel:
    def __init__(self, i, j):
        self.x = int(round(wPixel * (i + .5)))
        self.y = int(round(maxPixel * ( j + .5)))
        self.cpos = (self.x, self.y)
        self.rect = (self.x - cPixel * .5, self.y - cPixel * .5, cPixel, cPixel)
        self.color = (0,0,0)

    def remanence(self, c, cd):
        return [(c[i] * REM + cd[i] * (1 - REM)) for i in range(3)]

    def draw(self, color, screen):
        r, g, b = (e/255. for e in self.color)
        lum = math.sqrt( 0.299 * r**2 + 0.587 * g**2 + 0.114 * b**2 )
        
        mul = 2 * (lum ** ((1. / GAMMA) - 1) if lum > 0 else 1)

        color = [min(c * mul, 255) for c in color]
        self.color = self.remanence(self.color, color)
        color2 = [min(c * 1.5, 255) for c in self.color]

        r = int(round(.5 * (minPixel + (maxPixel - minPixel) * lum)))
        pygame.draw.circle(screen, self.color, self.cpos, r)

        pygame.draw.rect(screen, color2, self.rect)

#----------------------------------------------------------------
class NeoPixel:
    def __init__(self):
        self.running = False
        self.nb = {} 
        self.pixels = {}
        self.W = 0
        self.H = 0

    def initPixels(self, nb, row):
        print 'Row %d has %d pixels' %(row, nb)
        self.nb[row] = nb

        self.pixels[row] = []
        for i in range(nb):
            p = Pixel(i, row)
            self.pixels[row].append(p)

        self.initDisplay(nb, row)

    def initDisplay(self, n, row):
        self.W = max(self.W, n * wPixel)
        self.H = max(self.H, maxPixel * (row + 1))

        posx, posy = 1920 / 2 - self.W / 2, 1200 - self.H
        os.environ['SDL_VIDEO_WINDOW_POS'] = '%i,%i' % (posx,posy)

        self.screen = pygame.display.set_mode((self.W, self.H), pygame.NOFRAME | pygame.RESIZABLE)
        hwnd = pygame.display.get_wm_info()['window'] # handle to the window
        SetWindowPos(hwnd, TOPMOST, 0, 0, 0, 0, NOMOVE|NOSIZE)

        self.running = True

    def write(self, buf): 
        nb = (len(buf) - 1) / 3
        row = buf[0]
        if self.nb.get(row, 0) != nb:
            self.initPixels(nb, row)
            
        if self.running:
            #buf = bytearray(buf)

            if row == 0:
                self.screen.fill((0, 0, 0))
                
            for i, p in enumerate(self.pixels[row]):
                pos = 1 + i * 3
                p.draw((buf[pos], buf[pos + 1], buf[pos + 2]), self.screen)

            if row == len(self.nb) - 1:
                pygame.display.flip()

            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    self.running = False

#----------------------------------------------------------------
from zeroconf import ServiceBrowser, Zeroconf
import socket

class findEsp32:
    esp32Type = '_leds._tcp.local.'

    def add_service(self, zeroconf, typ, name):
        info = zeroconf.get_service_info(typ, name)
        if info and typ == self.esp32Type: 
            address =  'ws://%s:%d/' % (socket.inet_ntoa(info.address), info.port)
            name = info.name.split('.')[0]
            print 'found %s' % name
            self.whoToCallBack(address, name)

    def __init__(self, whoToCallBack):
        self.whoToCallBack = whoToCallBack
        self.zeroconf = Zeroconf()
        try:
            browser = ServiceBrowser(self.zeroconf, self.esp32Type, self)
        except:
            sys.stdout.write(traceback.format_exc())
            
    def close(self):
        self.zeroconf.close()

#-----------
import sys
import websocket

class Showled:

    np = NeoPixel()

    def onMessage(self, ws, message):
        self.np.write(bytearray(message))

    def onError(self, ws, error):
        pass #print(error)

    def onClose(self, ws):
        print 'disconnected'

    def onOpen(self, ws):
        print 'connected'

    def onEsp32Found(self, address, name):
        print 'try to connect to %s' % address
        ws = websocket.WebSocketApp(
            address,
            on_message = lambda ws,msg: self.onMessage(ws, msg),
            on_error   = lambda ws,msg: self.onError(ws, msg),
            on_close   = lambda ws:     self.onClose(ws),
            on_open    = lambda ws:     self.onOpen(ws))

        while True:
            ws.run_forever(ping_interval=3, ping_timeout=0)

    def __init__(self):
        self.browser = findEsp32(lambda addr, name : self.onEsp32Found(addr, name))

    def close(sel):
        self.browser(close)

#-----------
import time

Showled()
while True:
    time.sleep(1)
 
