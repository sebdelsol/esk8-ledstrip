#pragma once

// call a macro that calls a _m macro for each arg (max 9), 
// and _mLast for the last arg

#define  _M1(_m, _p, x)        _m##Last(_p, x)
#define  _M2(_m, _p, x, ...)   _m(_p, x)    _M1(_m, _p, __VA_ARGS__)
#define  _M3(_m, _p, x, ...)   _m(_p, x)    _M2(_m, _p, __VA_ARGS__)
#define  _M4(_m, _p, x, ...)   _m(_p, x)    _M3(_m, _p, __VA_ARGS__)
#define  _M5(_m, _p, x, ...)   _m(_p, x)    _M4(_m, _p, __VA_ARGS__)
#define  _M6(_m, _p, x, ...)   _m(_p, x)    _M5(_m, _p, __VA_ARGS__)
#define  _M7(_m, _p, x, ...)   _m(_p, x)    _M6(_m, _p, __VA_ARGS__)
#define  _M8(_m, _p, x, ...)   _m(_p, x)    _M7(_m, _p, __VA_ARGS__)
#define  _M9(_m, _p, x, ...)   _m(_p, x)    _M8(_m, _p, __VA_ARGS__)
#define _M10(_m, _p, x, ...)   _m(_p, x)    _M9(_m, _p, __VA_ARGS__)

#define _MNth(n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, N, ...) _M##N
#define _Mn(...) _MNth(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

#define MacroForEach(_m, _p, ...) _Mn(__VA_ARGS__)(_m, _p, __VA_ARGS__)

// -----------------------------------------------------
// create a join by separator
#include <Streaming.h>
#define _Join(sep, txt)     txt << sep <<
#define _JoinLast(sep, txt) txt
#define JoinbySpace(...)    MacroForEach(_Join, " ", __VA_ARGS__)

// create JoinbySpace(stream, ...)
//  inline Print& JoinbySpace(Print& stream) { return stream; }
// template<class T> 
//  inline Print& JoinbySpace(Print& stream, T arg) { return stream << arg; }
// template<class T, class... Args> 
//  inline Print& JoinbySpace(Print& stream, T first, Args... args) 
//  { 
//    stream << first << " ";
//    return JoinbySpace(stream, args...); 
//  }

// -----------------------------------------------------
// create the _forEach(...) method that calls _method(arg) for each arg
#define _MAP(_forEach, _method)                       \
    inline void _forEach() {};                        \
  template<class First, class... Rest>                \
    inline void _forEach(First& first, Rest&... rest) \
    {                                                 \
      _method(first);                                 \
      _forEach(rest...);                              \
    };
