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
SRV_IP = '192.168.0.10'
SRV_PORT = 81

#----------------------------------------------------------------
wPixel = 20 #50
cPixel = wPixel * .25 
minPixel = wPixel * .9
maxPixel = int(round(wPixel * 1.5))
REM = 0.5

#----------------------------------------------------------------
class Pixel:
    def __init__(self, i, j):
        self.x = int(round(wPixel * (i + .5)))
        self.y = int(round(maxPixel * ( j + .5)))
        self.cpos = (int(round(wPixel * .5)), int(round(maxPixel * .5)))
        self.blitpos = (wPixel * i, maxPixel * j)
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
        screen.blit(srfbuf, self.blitpos) #, special_flags=pygame.BLEND_RGB_ADD )
        color2 = [min(c*2, 255) for c in self.color]
        pygame.draw.rect(screen, color2, self.rect)

#----------------------------------------------------------------
class NeoPixel:
    def __init__(self):
        self.running = False
        self.nb = {} #0
        self.pixels = {}
        self.W = 0
        self.H = 0

    def initPixels(self, nb, row):
        print 'INIT', nb, row
        self.nb[row] = nb

        self.pixels[row] = []
        for i in range(nb):
            p = Pixel(i, row)
            self.pixels[row].append(p)

        self.initDisplay(nb, row)

    def initDisplay(self, n, row):
        self.W = max(self.W, n * wPixel)
        self.H = max(self.H, maxPixel * (row + 1))
        #print self.W, self.H

        posx, posy = 1920 / 2 - self.W / 2, 1200 - self.H
        os.environ['SDL_VIDEO_WINDOW_POS'] = '%i,%i' % (posx,posy)

        self.screen = pygame.display.set_mode((self.W,self.H),pygame.NOFRAME|pygame.RESIZABLE)
        self.srfbuf = pygame.Surface((wPixel, maxPixel))
        self.srfbuf.set_colorkey((0,0,0))

        hwnd = pygame.display.get_wm_info()['window'] # handle to the window
        SetWindowPos(hwnd, TOPMOST, 0, 0, 0, 0, NOMOVE|NOSIZE)

        self.clock = pygame.time.Clock()
        self.running = True

    def write(self, buf, nb, row):
        if nb==len(buf)/3: #check
            if self.nb.get(row, 0) != nb:
                self.initPixels(nb, row)
                
            if self.running:
                buf = bytearray(buf)

                if row == 0:
                    self.screen.fill((0,0,0))

                '''
                DATA = (hex(int((buf[i*3] + buf[i*3+1] + buf[i*3+2])/3)) for i, p in enumerate(self.pixels[row]))
                if row == 2:
                    print nb, row, ' '.join(DATA)
                '''
                    
                for i, p in enumerate(self.pixels[row]):
                    pos = i * 3
                    p.draw((buf[pos], buf[pos+1], buf[pos+2]), self.screen, self.srfbuf)

                if row == len(self.nb)-1:
                    pygame.display.flip()
                #self.clock.tick(60)
                #sys.stdout.write('%02f\r'%self.clock.get_fps())

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
            #print self.data
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
        print(self.address, 'connected')

    def handleClose(self):
        print(self.address, 'closed')

def main():
    
    server = SimpleWebSocketServer(SRV_IP, SRV_PORT, Showled)
    print 'server started'
    server.serveforever()

main()
