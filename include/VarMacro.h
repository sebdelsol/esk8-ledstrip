#pragma once

#define _m1(_p, _p2, _m, _lm, x)        _lm(_p, _p2, x)
#define _m2(_p, _p2, _m, _lm, x, ...)   _m(_p, _p2, x) _m1(_p, _p2, _m, _lm, __VA_ARGS__)
#define _m3(_p, _p2, _m, _lm, x, ...)   _m(_p, _p2, x) _m2(_p, _p2, _m, _lm, __VA_ARGS__)
#define _m4(_p, _p2, _m, _lm, x, ...)   _m(_p, _p2, x) _m3(_p, _p2, _m, _lm, __VA_ARGS__)
#define _m5(_p, _p2, _m, _lm, x, ...)   _m(_p, _p2, x) _m4(_p, _p2, _m, _lm, __VA_ARGS__)
#define _m6(_p, _p2, _m, _lm, x, ...)   _m(_p, _p2, x) _m5(_p, _p2, _m, _lm, __VA_ARGS__)
#define _m7(_p, _p2, _m, _lm, x, ...)   _m(_p, _p2, x) _m6(_p, _p2, _m, _lm, __VA_ARGS__)
#define _m8(_p, _p2, _m, _lm, x, ...)   _m(_p, _p2, x) _m7(_p, _p2, _m, _lm, __VA_ARGS__)
#define _m9(_p, _p2, _m, _lm, x, ...)   _m(_p, _p2, x) _m8(_p, _p2, _m, _lm, __VA_ARGS__)
#define _m10(_p, _p2, _m, _lm, x, ...)  _m(_p, _p2, x) _m9(_p, _p2, _m, _lm, __VA_ARGS__)

#define _GetNthArg(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
#define _GetM(...) _GetNthArg( __VA_ARGS__, _m10, _m9, _m8, _m7, _m6, _m5, _m4, _m3, _m2, _m1)

#define CallMacroForEachL(_p, _p2, _m, _lm, ...) _GetM(__VA_ARGS__)(_p, _p2, _m, _lm, __VA_ARGS__)
#define CallMacroForEach(_p, _p2, _m, ...)       _GetM(__VA_ARGS__)(_p, _p2, _m, _m, __VA_ARGS__)

// -----------------------------------------------------
#define _Join(__, sep, txt)     txt << sep <<
#define _JoinLast(__, sep, txt) txt
#define JoinbySpace(...)        CallMacroForEachL(__, " ", _Join, _JoinLast, __VA_ARGS__)