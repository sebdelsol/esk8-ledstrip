#pragma once

#define _f1(_G, _G2, _f, _lf, x)      _lf(_G, _G2, x)
#define _f2(_G, _G2, _f, _lf, x, ...) _f(_G, _G2, x) _f1(_G, _G2, _f, _lf, __VA_ARGS__)
#define _f3(_G, _G2, _f, _lf, x, ...) _f(_G, _G2, x) _f2(_G, _G2, _f, _lf, __VA_ARGS__)
#define _f4(_G, _G2, _f, _lf, x, ...) _f(_G, _G2, x) _f3(_G, _G2, _f, _lf, __VA_ARGS__)
#define _f5(_G, _G2, _f, _lf, x, ...) _f(_G, _G2, x) _f4(_G, _G2, _f, _lf, __VA_ARGS__)
#define _f6(_G, _G2, _f, _lf, x, ...) _f(_G, _G2, x) _f5(_G, _G2, _f, _lf, __VA_ARGS__)
#define _f7(_G, _G2, _f, _lf, x, ...) _f(_G, _G2, x) _f6(_G, _G2, _f, _lf, __VA_ARGS__)
#define _f8(_G, _G2, _f, _lf, x, ...) _f(_G, _G2, x) _f7(_G, _G2, _f, _lf, __VA_ARGS__)
#define _f9(_G, _G2, _f, _lf, x, ...) _f(_G, _G2, x) _f8(_G, _G2, _f, _lf, __VA_ARGS__)

#define _GetNthArg(_1, _2, _3, _4, _5, _6, _7, _8, _9, N, ...) N
#define _GetF(...) _GetNthArg( __VA_ARGS__, _f9, _f8, _f7, _f6, _f5, _f4, _f3, _f2, _f1)

#define CallMacroForEachL(_G, _G2, _f, _lf, ...) _GetF(__VA_ARGS__)(_G, _G2, _f, _lf, __VA_ARGS__)
#define CallMacroForEach(_G, _G2, _f, ...)       _GetF(__VA_ARGS__)(_G, _G2, _f, _f, __VA_ARGS__)

// -----------------------------------------------------
#define _Join(__, sep, txt)     txt << sep <<
#define _JoinLast(__, sep, txt) txt
#define JoinbySpace(...)        CallMacroForEachL(__, " ", _Join, _JoinLast, __VA_ARGS__)