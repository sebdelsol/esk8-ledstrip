#pragma once

#define _m1(_p, _p2, _m, _lm, x)        _lm(_p, _p2, x)
#define _m2(_p, _p2, _m, _lm, x, ...)   _m(_p, _p2, x)    _m1(_p, _p2, _m, _lm, __VA_ARGS__)
#define _m3(_p, _p2, _m, _lm, x, ...)   _m(_p, _p2, x)    _m2(_p, _p2, _m, _lm, __VA_ARGS__)
#define _m4(_p, _p2, _m, _lm, x, ...)   _m(_p, _p2, x)    _m3(_p, _p2, _m, _lm, __VA_ARGS__)
#define _m5(_p, _p2, _m, _lm, x, ...)   _m(_p, _p2, x)    _m4(_p, _p2, _m, _lm, __VA_ARGS__)
#define _m6(_p, _p2, _m, _lm, x, ...)   _m(_p, _p2, x)    _m5(_p, _p2, _m, _lm, __VA_ARGS__)
#define _m7(_p, _p2, _m, _lm, x, ...)   _m(_p, _p2, x)    _m6(_p, _p2, _m, _lm, __VA_ARGS__)
#define _m8(_p, _p2, _m, _lm, x, ...)   _m(_p, _p2, x)    _m7(_p, _p2, _m, _lm, __VA_ARGS__)
#define _m9(_p, _p2, _m, _lm, x, ...)   _m(_p, _p2, x)    _m8(_p, _p2, _m, _lm, __VA_ARGS__)
#define _m10(_p, _p2, _m, _lm, x, ...)  _m(_p, _p2, x)    _m9(_p, _p2, _m, _lm, __VA_ARGS__)

#define _GetNth_m(n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, N, ...) _m##N
#define _Get_m(...) _GetNth_m(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

#define CallMacroForEachL(_p, _p2, _m, _lm, ...) _Get_m(__VA_ARGS__)(_p, _p2, _m, _lm, __VA_ARGS__)
#define CallMacroForEach(_p, _p2, _m, ...)       _Get_m(__VA_ARGS__)(_p, _p2, _m, _m, __VA_ARGS__)

// -----------------------------------------------------
#define _Join(__, sep, txt)     txt << sep <<
#define _JoinLast(__, sep, txt) txt
#define JoinbySpace(...)        CallMacroForEachL(__, " ", _Join, _JoinLast, __VA_ARGS__)