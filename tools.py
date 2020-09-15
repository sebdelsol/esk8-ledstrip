import socket
import time
import re

#---------------------------------------------------
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

#---------------------------------------------------
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
