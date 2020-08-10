#pragma once
#include <Streaming.h>

#define  _M1(_m, _p, _p2, x)        _m##Last(_p, _p2, x)
#define  _M2(_m, _p, _p2, x, ...)   _m(_p, _p2, x)    _M1(_m, _p, _p2, __VA_ARGS__)
#define  _M3(_m, _p, _p2, x, ...)   _m(_p, _p2, x)    _M2(_m, _p, _p2, __VA_ARGS__)
#define  _M4(_m, _p, _p2, x, ...)   _m(_p, _p2, x)    _M3(_m, _p, _p2, __VA_ARGS__)
#define  _M5(_m, _p, _p2, x, ...)   _m(_p, _p2, x)    _M4(_m, _p, _p2, __VA_ARGS__)
#define  _M6(_m, _p, _p2, x, ...)   _m(_p, _p2, x)    _M5(_m, _p, _p2, __VA_ARGS__)
#define  _M7(_m, _p, _p2, x, ...)   _m(_p, _p2, x)    _M6(_m, _p, _p2, __VA_ARGS__)
#define  _M8(_m, _p, _p2, x, ...)   _m(_p, _p2, x)    _M7(_m, _p, _p2, __VA_ARGS__)
#define  _M9(_m, _p, _p2, x, ...)   _m(_p, _p2, x)    _M8(_m, _p, _p2, __VA_ARGS__)
#define _M10(_m, _p, _p2, x, ...)   _m(_p, _p2, x)    _M9(_m, _p, _p2, __VA_ARGS__)

#define _MNth(n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, N, ...) _M##N
#define _Mn(...) _MNth(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

#define CallMacroForEach(_m, _p, _p2, ...) _Mn(__VA_ARGS__)(_m, _p, _p2, __VA_ARGS__)

// -----------------------------------------------------
// #define _Join(__, sep, txt)     txt << sep <<
// #define JoinbySpace(...)        CallMacroForEach(_Join, __, " ", __VA_ARGS__)
// #define _JoinLast(__, sep, txt) txt


// void JoinbySpace() {}
// template<typename T, typename... Args> void JoinbySpace(T& first, Args&... args) 
// { 
//   Serial << first << " "; 
//   JoinbySpace(args...); 
// }

// -----------------------------------------------------
inline Print& JoinbySpace(Print& stream) { return stream; }
inline Print& JoinbySpace(Print& stream, T arg) { return stream << arg; }
template<class T, class... Args> inline Print& JoinbySpace(Print& stream, T first, Args... args) 
{ 
  return stream << first << " " << JoinbySpace(stream, args...); 
}

// -----------------------------------------------------
//#define _MAP(_name, _func)                                                                       \
//  template<class P> inline void _##_name(P& p) {}                                                \
//  template<class P, class T, class... Args> inline void _##_name(P& p, T& first, Args&... args)  \
//  {                                                                                              \
//    _func(p, first);                                                                             \
//    _##_name(p, args...);                                                                        \
//  }
// -----------------------------------------------------
#define _MAP(_name, _func)                                                                       \
  inline void _##_name() {}                                                \
  template<class First, class... Args> inline void _##_name(First& first, Args&... args)  \
  {                                                                                              \
    _func(first);                                                                             \
    _##_name(args...);                                                                        \
  }
