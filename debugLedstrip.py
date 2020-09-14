import os
import traceback

#----------------------------------------------------------------
import re

def findInFile(fname, split, *search):
    found = []
    search = list(search)
    
    with open(fname, "r") as f:
        for l in f.readlines():
            if len(search) == 0: break
            w = re.sub(' +', ' ', l).split(split)
            if len(w) > 1:
                w = map(lambda x: x.replace(' ', ''), w)
                for s in search:
                    if s in w:
                        found.append(re.sub(r'[\n"\']', '', w[w.index(s) + 1]))
                        search.remove(s)
                        break
                    
    return found[0] if len(found)==1 else found

#----------------------------------------------------------------
wPixel = 25
cPixel = wPixel * .25 
minPixel = wPixel * .6
maxPixel = int(round(wPixel * 1.))
REM = 0.5
GAMMA = 5.

#----------
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
os.environ['PYGAME_HIDE_SUPPORT_PROMPT'] = "hide"
import pygame

from ctypes import windll
TOPMOST, NOSIZE, NOMOVE = -1, 1, 2

class Strip:

    def __init__(self):
        self.running = False
        self.n = {} 
        self.pixels = {}
        self.W = 0
        self.H = 0
        self.t = time.time()

        tick = findInFile('./include/cfg.h', ' ', 'WIFI_TICK')
        self.tick = int(tick) / 1000.

    def initPixels(self, strip, n):
        print 'add strip with %d pixels' % n
        self.n[strip] = n
        self.pixels[strip] = [Pixel(i, strip) for i in range(n)]
        self.initDisplay(n, strip)

    def initDisplay(self, n, strip):
        self.W = max(self.W, n * wPixel)
        self.H = max(self.H, maxPixel * (strip + 1))

        # SCREENW, SCREENH = 1920, 1200
        # pos = (SCREENW / 2 - self.W / 2, SCREENH - self.H)
        # os.environ['SDL_VIDEO_WINDOW_POS'] = '%i,%i' % pos

        self.screen = pygame.display.set_mode((self.W, self.H)) # pygame.NOFRAME
        hwnd = pygame.display.get_wm_info()['window'] # handle to the window
        windll.user32.SetWindowPos(hwnd, TOPMOST, 0, 0, 0, 0, NOMOVE|NOSIZE)

        self.running = True

    def write(self, buf, length, strip): 
        n = length / 3
        
        if self.n.get(strip, 0) != n:
            self.initPixels(strip, n)
            
        if self.running:
            if strip == 0:
                self.screen.fill((0, 0, 0))
                
            for i, p in enumerate(self.pixels[strip]):
                pos = i * 3
                p.draw((buf[pos], buf[pos + 1], buf[pos + 2]), self.screen)

            if strip == len(self.n) - 1:
                dt = time.time() - self.t
                time.sleep(max(0, self.tick - dt))
                self.t = time.time()
                pygame.display.flip()

            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    self.running = False

#----------------------------------------------------------------
from threading import Thread
import socket
import time

def findServerAddr(callback):

    hostname, port = findInFile('./platformio.ini', '=', 'otaname', 'otaport')
    hostname = '%s.local' % hostname
    port = int(port)

    print 'seek %s' % hostname
    try:
        ip = socket.gethostbyname(hostname)
        print 'found %s' % hostname
        callback((ip, port))

    except socket.gaierror:
        time.sleep(1)

#-----------
import struct

class Showled:

    def recvMsg(self):
        header = self.recvn(2)
        if not header: return None
        length, strip = struct.unpack('BB', header)
        buf = self.recvn(length)
        return buf, length, strip if buf else None

    def recvn(self, n):
        buf = bytearray()
        while len(buf) < n:
            packet = self.sock.recv(n - len(buf))
            if not packet: return None
            buf.extend(packet)
        return buf

    def onServerFound(self, address):
        self.np = Strip()
        
        while True:
            print 'connecting to %s:%d' % address
            self.sock = socket.socket() 
            self.sock.connect(address)
            self.sock.settimeout(3)
            
            connected = True
            while connected:
                try:
                    msg = self.recvMsg()
                    if msg is not None:
                        self.np.write(*msg)
                    else:
                        connected = False
                
                except socket.timeout:
                    traceback.print_exc()
                    connected = False
            
            print 'disconnected'
            self.sock.close()

    def __init__(self):
        callback = lambda addr : self.onServerFound(addr)
        Thread(target = findServerAddr, args = (callback, )).start()

#-----------
Showled() 
