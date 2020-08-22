N = 20

''' for N = 2

#define  _M1(_m, _p, x)        _m(_p, x)
#define  _M2(_m, _p, x, ...)   _m(_p, x)    _M1(_m, _p, __VA_ARGS__)
#define _MNth(n1, n2, N, ...) _M##N
#define _Mn(...) _MNth(__VA_ARGS__, 2, 1)
'''

with open('./include/varMac.h', 'w') as f:  
    for i in range(1, N+1):
        if i == 1:
            f.write("#define _M%d(_m, _p, x) \t\t\t\t _m(_p, x)\n" %i)
        else:
            f.write("#define _M%d(_m, _p, x, ...) \t _m(_p, x)  _M%d(_m, _p, __VA_ARGS__)\n" %(i, i-1))

    f.write("\n")
    
    args = ", ".join(("n%d"%i for i in range(1, N+1)))
    f.write("#define _MNth(%s, N, ...) _M##N\n" %(args))

    args = ", ".join(("%d"%i for i in range(N, 0, -1)))
    f.write("#define _Mn(...) _MNth(__VA_ARGS__, %s)\n" %(args))
