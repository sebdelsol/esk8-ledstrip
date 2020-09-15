from tools import findServerAddr
from threading import Thread
import struct
import socket
import time
import traceback

class Log:

    def onServerFound(self, address):

        address = (address[0], 23) # telnet port        

        while True:
            print 'connecting to %s:%d' % address
            self.sock = socket.socket() 
            self.sock.settimeout(None)
            self.sock.connect(address)
            self.sock.settimeout(3)
            print '--------------'
            
            connected = True
            while connected:
                try:
                    buf = self.sock.recv(4096)

                    if buf is not None:
                        if len(buf) > 0 and buf[0] is not '\0':
                            print buf[:-1] if buf[-1]=='\n' else buf
                    else:
                        connected = False
            
                except socket.timeout:
                    #traceback.print_exc()
                    connected = False
            
            print '--------------'
            print 'disconnected'
            self.sock.close()

    def __init__(self):
        callback = lambda addr : self.onServerFound(addr)
        Thread(target = findServerAddr, args = (callback, )).start()

Log()
