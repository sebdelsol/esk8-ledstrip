#pragma once

#define _f1(_G, _c, _lc, x)      _lc(_G, x)
#define _f2(_G, _c, _lc, x, ...) _c(_G, x) _f1(_G, _c, _lc, __VA_ARGS__)
#define _f3(_G, _c, _lc, x, ...) _c(_G, x) _f2(_G, _c, _lc, __VA_ARGS__)
#define _f4(_G, _c, _lc, x, ...) _c(_G, x) _f3(_G, _c, _lc, __VA_ARGS__)
#define _f5(_G, _c, _lc, x, ...) _c(_G, x) _f4(_G, _c, _lc, __VA_ARGS__)
#define _f6(_G, _c, _lc, x, ...) _c(_G, x) _f5(_G, _c, _lc, __VA_ARGS__)
#define _f7(_G, _c, _lc, x, ...) _c(_G, x) _f6(_G, _c, _lc, __VA_ARGS__)
#define _f8(_G, _c, _lc, x, ...) _c(_G, x) _f7(_G, _c, _lc, __VA_ARGS__)
#define _f9(_G, _c, _lc, x, ...) _c(_G, x) _f8(_G, _c, _lc, __VA_ARGS__)

#define _GetNthArg(_1, _2, _3, _4, _5, _6, _7, _8, _9, N, ...) N
#define _GetF(...) _GetNthArg( __VA_ARGS__, _f9, _f8, _f7, _f6, _f5, _f4, _f3, _f2, _f1)

#define CallMacroForEachL(_G, _c, _lc, ...) _GetF(__VA_ARGS__)(_G, _c, _lc, __VA_ARGS__)
#define CallMacroForEach(_G, _c, ...)       CallMacroForEachL(_G, _c, _c, __VA_ARGS__)

// -----------------------------------------------------
#define _AddSpace(sep, txt)     txt << sep <<
#define _LastAddSpace(sep, txt) txt
#define SEPbySPACE(...)         CallMacroForEachL(" ", _AddSpace, _LastAddSpace, __VA_ARGS__)
