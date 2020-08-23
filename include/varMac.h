// created by createVarMac.py 
// for N=20

#pragma once

#define _M1(_m, _p, x) 				 _m(_p, x)
#define _M2(_m, _p, x, ...) 	 _m(_p, x)  _M1(_m, _p, __VA_ARGS__)
#define _M3(_m, _p, x, ...) 	 _m(_p, x)  _M2(_m, _p, __VA_ARGS__)
#define _M4(_m, _p, x, ...) 	 _m(_p, x)  _M3(_m, _p, __VA_ARGS__)
#define _M5(_m, _p, x, ...) 	 _m(_p, x)  _M4(_m, _p, __VA_ARGS__)
#define _M6(_m, _p, x, ...) 	 _m(_p, x)  _M5(_m, _p, __VA_ARGS__)
#define _M7(_m, _p, x, ...) 	 _m(_p, x)  _M6(_m, _p, __VA_ARGS__)
#define _M8(_m, _p, x, ...) 	 _m(_p, x)  _M7(_m, _p, __VA_ARGS__)
#define _M9(_m, _p, x, ...) 	 _m(_p, x)  _M8(_m, _p, __VA_ARGS__)
#define _M10(_m, _p, x, ...) 	 _m(_p, x)  _M9(_m, _p, __VA_ARGS__)
#define _M11(_m, _p, x, ...) 	 _m(_p, x)  _M10(_m, _p, __VA_ARGS__)
#define _M12(_m, _p, x, ...) 	 _m(_p, x)  _M11(_m, _p, __VA_ARGS__)
#define _M13(_m, _p, x, ...) 	 _m(_p, x)  _M12(_m, _p, __VA_ARGS__)
#define _M14(_m, _p, x, ...) 	 _m(_p, x)  _M13(_m, _p, __VA_ARGS__)
#define _M15(_m, _p, x, ...) 	 _m(_p, x)  _M14(_m, _p, __VA_ARGS__)
#define _M16(_m, _p, x, ...) 	 _m(_p, x)  _M15(_m, _p, __VA_ARGS__)
#define _M17(_m, _p, x, ...) 	 _m(_p, x)  _M16(_m, _p, __VA_ARGS__)
#define _M18(_m, _p, x, ...) 	 _m(_p, x)  _M17(_m, _p, __VA_ARGS__)
#define _M19(_m, _p, x, ...) 	 _m(_p, x)  _M18(_m, _p, __VA_ARGS__)
#define _M20(_m, _p, x, ...) 	 _m(_p, x)  _M19(_m, _p, __VA_ARGS__)

#define _MNth(n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11, n12, n13, n14, n15, n16, n17, n18, n19, n20, N, ...) _M##N
#define _Mn(...) _MNth(__VA_ARGS__, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)
