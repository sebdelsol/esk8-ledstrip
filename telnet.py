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
        try:
            sock = socket.socket() 
            sock.settimeout(10)
            sock.connect(address)
            print('connected to %s:%d' % address)
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
        except :
            #traceback.print_exc()
            sock.close()
            time.sleep(2)


Thread(target = findServerAddr, args = (onServerFound, )).start()
while True: time.sleep(1)
