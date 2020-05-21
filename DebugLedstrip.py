import pygame
import math
import os
import traceback

from ctypes import windll
SetWindowPos = windll.user32.SetWindowPos
TOPMOST = -1
NOSIZE = 1
NOMOVE = 2

#---------------------------------------------------------------
with open("./include/wificonfig.h", "r") as f:
    for l in f.readlines():
        w = l.split(" ")
        if len(w)>2:
            if w[1] == "SOCK_ADDR":
                SOCK_ADDR = w[2].replace('\n', '').replace('"', '')
            elif w[1] == "SOCK_PORT":
                SOCK_PORT = int(w[2])

#----------------------------------------------------------------
wPixel = 25
cPixel = wPixel * .25 
minPixel = wPixel * .6
maxPixel = int(round(wPixel * 1.))
REM = 0.5
GAMMA = 5

#----------------------------------------------------------------
class Pixel:
    def __init__(self, i, j):
        self.x = int(round(wPixel * (i + .5)))
        self.y = int(round(maxPixel * ( j + .5)))
        self.cpos = (self.x, self.y)
        self.rect = (self.x - cPixel * .5, self.y - cPixel * .5, cPixel, cPixel)
        self.color = (0,0,0)

    def remanence(self, c, cd):
        return cd if cd >= c else c * REM + cd * (1 - REM)

    def draw(self, color, screen):
        r, g, b = (e/255. for e in self.color)
        lum = math.sqrt( 0.299 * r**2 + 0.587 * g**2 + 0.114 * b**2 )
        mul = 2 * (lum ** ((1./GAMMA) - 1) if lum> 0 else 1)
        color = (min(color[0] * mul, 255), min(color[1] * mul, 255), min(color[2] * mul, 255))

        self.color = (self.remanence(self.color[0], color[0]),
                      self.remanence(self.color[1], color[1]),
                      self.remanence(self.color[2], color[2]))

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

    def write(self, buf, nb, row):
        if nb==len(buf) / 3:
            if self.nb.get(row, 0) != nb:
                self.initPixels(nb, row)
                
            if self.running:
                buf = bytearray(buf)

                if row == 0:
                    self.screen.fill((0, 0, 0))
                    
                for i, p in enumerate(self.pixels[row]):
                    pos = i * 3
                    p.draw((buf[pos], buf[pos + 1], buf[pos + 2]), self.screen)

                if row == len(self.nb) - 1:
                    pygame.display.flip()

                for event in pygame.event.get():
                    if event.type == pygame.QUIT:
                        self.running = False

#----------------------------------------------------------------
import sys
from SimpleWebSocketServer import SimpleWebSocketServer, WebSocket

TEXT = 0x1
BINARY = 0x2

class Showled(WebSocket):

    np = NeoPixel()
    currentNo = None;
    currentNb = None;

    def handleMessage(self):
        try:
            if self.opcode==BINARY:
                self.np.write(self.data, self.currentNb, self.currentNo)

            elif self.opcode==TEXT:
                info = self.data.split(' ')
                if len(info)==3 and info[0]=='STRIP':
                    self.currentNo = int(info[1])
                    self.currentNb = int(info[2])

        except:
            sys.stdout.write(traceback.format_exc())

    def handleConnected(self):
        print 'Connected @%s:%s' %self.address

    def handleClose(self):
        print 'Closed @%s:%s' %self.address

def main():
    
    print "Starting server @%s:%d" % (SOCK_ADDR, SOCK_PORT)
    server = SimpleWebSocketServer(SOCK_ADDR, SOCK_PORT, Showled)
    print 'Server started'
    server.serveforever()

main()
