import os 

N = 20
fName = './include/varMac.h'

''' 

// for N = 2

#define _M1(_m, _p, x)        _m(_p, x)
#define _M2(_m, _p, x, ...)   _m(_p, x)    _M1(_m, _p, __VA_ARGS__)

#define _MNth(n1, n2, N, ...) _M##N
#define _Mn(...) _MNth(__VA_ARGS__, 2, 1)

'''

with open(fName, 'w') as f:  
    def addLine(txt = ""): 
        f.write("%s\n" % txt)

    addLine("// created by %s " % os.path.basename(__file__))
    addLine("// for N=%d" % N)

    addLine()
    addLine("#pragma once")
    addLine()

    addLine("#define _M1(_m, _p, x) \t\t\t\t _m(_p, x)")
    
    for i in range(2, N+1):
        addLine("#define _M%d(_m, _p, x, ...) \t _m(_p, x)  _M%d(_m, _p, __VA_ARGS__)" % (i, i-1))

    addLine()
    
    args = ", ".join(("n%d" % i for i in range(1, N+1)))
    addLine("#define _MNth(%s, N, ...) _M##N" % args)

    args = ", ".join(("%d" % i for i in range(N, 0, -1)))
    addLine("#define _Mn(...) _MNth(__VA_ARGS__, %s)" % args)
