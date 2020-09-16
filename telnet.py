from __future__ import print_function
from tools import findServerAddr
from threading import Thread
import struct
import socket
import time
import traceback

print ('           TELNET')
print ('---------------------------')

def onServerFound(address):

    address = (address[0], 23) # telnet port        

    while True:
        print('connecting to %s:%d' % address)
        sock = socket.socket() 
        sock.settimeout(None)
        sock.connect(address)
        sock.settimeout(3)
        print('---------------------------')
        
        connected = True
        while connected:
            try:
                buf = sock.recv(4096)

                if buf is not None:
                    if buf[0] is not '\0': # keep-alive
                        print(buf, end='') # '\n' already in buf
                else:
                    connected = False
        
            except socket.timeout:
                #traceback.print_exc()
                connected = False
        
        print('\n---------------------------')
        print('disconnected')
        sock.close()

Thread(target = findServerAddr, args = (onServerFound, )).start()
while True: time.sleep(1)
