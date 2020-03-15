import pygame
#import random
#import time
import math
import os
import traceback

from ctypes import windll
SetWindowPos = windll.user32.SetWindowPos
TOPMOST = -1
NOSIZE = 1
NOMOVE = 2

#----------------------------------------------------------------
wPixel = 20 #50
cPixel = wPixel * .25 
minPixel = wPixel * .9
maxPixel = int(round(wPixel * 1.5))
REM = 0.5

#----------------------------------------------------------------
class Pixel:
    def __init__(self, i):
        self.x = int(round(wPixel * (i + .5)))
        self.y = int(round(maxPixel * .5))
        self.cpos = (int(round(wPixel * .5)), int(round(maxPixel * .5)))
        self.blitpos = (wPixel * i, 0)
        self.rect = (self.x-cPixel * .5, self.y - cPixel * .5, cPixel, cPixel)
        self.color = (0,0,0)

    def remanence(self, c, cd):
        if cd >= c:
            return cd
        else:
            return c * REM + cd * (1 - REM)

    def draw(self, color, screen, srfbuf):
        self.color = (self.remanence(self.color[0], color[0]),
                      self.remanence(self.color[1], color[1]),
                      self.remanence(self.color[2], color[2]))
        r, g, b = (e/255. for e in self.color)
        lum = math.sqrt( 0.299*r**2 + 0.587*g**2 + 0.114*b**2 )

        r = int(round(.5 * (minPixel + (maxPixel-minPixel) * lum)))

        srfbuf.fill((0,0,0))
        #srfbuf.set_alpha(lum * 150)
        pygame.draw.circle(srfbuf, self.color, self.cpos, r)
        screen.blit(srfbuf, self.blitpos)#, special_flags=pygame.BLEND_RGB_ADD )
        color2 = [min(c*2, 255) for c in self.color]
        pygame.draw.rect(screen, color2, self.rect)

#----------------------------------------------------------------
class NeoPixel:
    def __init__(self):
        self.running = False
        self.n = 0

    def initPixels(self, n):
        self.n = n

        self.pixels = []
        for i in range(n):
            p = Pixel(i)
            self.pixels.append(p)

        self.initDisplay(n)

    def initDisplay(self, n):
        W, H = n * wPixel, maxPixel

        posx, posy = 1920 / 2 - W / 2, 1200 - H
        os.environ['SDL_VIDEO_WINDOW_POS'] = '%i,%i' % (posx,posy)

        self.screen = pygame.display.set_mode((W,H),pygame.NOFRAME|pygame.RESIZABLE)
        self.srfbuf = pygame.Surface((wPixel, maxPixel))
        self.srfbuf.set_colorkey((0,0,0))

        hwnd = pygame.display.get_wm_info()['window'] # handle to the window
        SetWindowPos(hwnd, TOPMOST, 0, 0, 0, 0, NOMOVE|NOSIZE)

        self.clock = pygame.time.Clock()
        self.running = True

    def write(self, buf):
        n = len(buf)/3
        if not self.running or n != self.n:
            self.initPixels(n)
            
        if self.running:
            buf = bytearray(buf)

            self.screen.fill((0,0,0))
            for i, p in enumerate(self.pixels):
                pos = i * 3
                p.draw((buf[pos], buf[pos+1], buf[pos+2]), self.screen, self.srfbuf)

            pygame.display.flip()
            #self.clock.tick(60)
            #sys.stdout.write('%02f\r'%self.clock.get_fps())

            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    self.running = False

#----------------------------------------------------------------
import sys
from SimpleWebSocketServer import SimpleWebSocketServer, WebSocket

class Showled(WebSocket):

    np = NeoPixel()

    def handleMessage(self):
        try:
            self.np.write(self.data)

        except:
            sys.stdout.write(traceback.format_exc())

    def handleConnected(self):
        print(self.address, 'connected')

    def handleClose(self):
        print(self.address, 'closed')

def main():
    
    server = SimpleWebSocketServer('192.168.0.10', 81, Showled)
    print 'server started'
    server.serveforever()

main()
