import os
import traceback

#---------------------------------------------------------------
with open("./platformio.ini", "r") as f:
    for l in f.readlines():
        w = l.replace(' ', '').split('=')
        if len(w)>1:
            if w[0] == "OTAname":
                SOCK_HOSTNAME = w[1].replace('\n', '').replace('"', '').replace("'", '')
            elif w[0] == "OTAport":
                SOCK_PORT = int(w[1])

#----------------------------------------------------------------
wPixel = 25
cPixel = wPixel * .25 
minPixel = wPixel * .6
maxPixel = int(round(wPixel * 1.))
REM = 0.5
GAMMA = 5.

#----------------------------------------------------------------
import pygame
import math

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
from ctypes import windll
SetWindowPos = windll.user32.SetWindowPos
TOPMOST, NOSIZE, NOMOVE = -1, 1, 2

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
        row = buf[0]
        nb = (len(buf) - 1) / 3
        
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
import socket
import threading
import time

class findSocketAddr(threading.Thread):
    
    def __init__(self, whoToCallBack):      
        self.whoToCallBack = whoToCallBack
        threading.Thread.__init__(self) 

    def run(self):
        name = '%s.local' % SOCK_HOSTNAME
        try:
            ip = socket.gethostbyname(name)
            print 'found %s' % name
            self.whoToCallBack('ws://%s:%d/' % (ip, SOCK_PORT))

        except socket.gaierror:
            time.sleep(1)

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
        if self.connected :
            print 'disconnected'
        self.connected = False

    def onOpen(self, ws):
        self.connected = True
        print 'connected'

    def onEsp32Found(self, address):
        print 'try to connect to %s' % address
        ws = websocket.WebSocketApp(address,
            on_message = lambda ws,msg: self.onMessage(ws, msg),
            on_error   = lambda ws,msg: self.onError(ws, msg),
            on_close   = lambda ws:     self.onClose(ws),
            on_open    = lambda ws:     self.onOpen(ws))

        self.connected = False
        while True:
            ws.run_forever(ping_interval=3, ping_timeout=0)

    def __init__(self):
        findSocketAddr(lambda addr : self.onEsp32Found(addr)).start()

#-----------
Showled()

while True:
    time.sleep(1)
 
