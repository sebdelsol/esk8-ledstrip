from __future__ import print_function
from tools import findServerAddr
from threading import Thread
import socket
import time
import traceback

sep = '-' * 30
print('TELNET')
print(sep)

def onServerFound(address):

    address = (address[0], 23) # telnet port        

    while True:
        print('connecting to %s:%d' % address)
        sock = socket.socket() 
        sock.settimeout(None)
        sock.connect(address)
        sock.settimeout(3)
        connected = True

        print(sep)
        print('\n')
        
        while connected:
            try:
                buf = sock.recv(4096)

                if buf is not None:
                    if len(buf) > 0 and buf[0] is not '\0': # keep-alive
                        print(buf, end='') # '\n' already in buf
                else:
                    connected = False
        
            except socket.timeout:
                #traceback.print_exc()
                connected = False
        
        sock.close()

        print ('\n')
        print(sep)
        print('disconnected')

Thread(target = findServerAddr, args = (onServerFound, )).start()
while True: time.sleep(1)
