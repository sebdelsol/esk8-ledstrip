#pragma once

// -----------------------------------------------------
// create a macro that calls a _m macro for each arg (max 9)

#define  _M1(_m, _p, x)        _m(_p, x)
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

#define ForEachMacro(_m, _p, ...) _Mn(__VA_ARGS__)(_m, _p, __VA_ARGS__)

// -----------------------------------------------------
// create JoinbySpace(stream, ...) 

inline Print& JoinbySpace(Print& stream) {}

template<class Last> 
inline Print& JoinbySpace(Print& stream, Last last) { return stream << last; }

template<class First, class... Args> 
inline Print& JoinbySpace(Print& stream, First first, Args... args) 
{ 
  stream << first << " ";
  return JoinbySpace(stream, args...); 
}

// -----------------------------------------------------
// create the _methods(...) that calls _method(arg) for each arg

#define _forEach(_method) _method##s

#define ForEachMethod(_method)                                \
  inline void _forEach(_method)() {};                         \
  template<class First, class... Args>                        \
  inline void _forEach(_method)(First& first, Args&... args)  \
  {                                                           \
    _method(first);                                           \
    _forEach(_method)(args...);                               \
  };
