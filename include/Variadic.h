#pragma once

#include <log.h>

// -----------------------------------------------------
#include <varMac.h> 
// run createVarMac.py to create all _Mn(_m, _ml, x) macros in varMac.h
// see N in createVarMac.py

// create a macro that calls a _m macro for each arg & _ml for the last arg
#define ForEachMacro(_m, _ml, ...) _Mn(__VA_ARGS__)(_m, _ml, __VA_ARGS__)

// create NameIt(...) // need @least one args...
#define _nameit(obj)     obj, #obj,
#define _nameitLast(obj) obj, #obj
#define NameIt(...)      ForEachMacro(_nameit, _nameitLast, __VA_ARGS__)

// create SpaceIt(...) // need @least one args...
#define _join(txt)       (txt) << F(" ") <<
#define _joinLast(txt)   (txt)
#define SpaceIt(...)     ForEachMacro(_join, _joinLast, __VA_ARGS__)

// create SpaceItHex(...) // need @least one args...
#define _joinH(txt)       _HEXS(txt) << F(" ") <<
#define _joinHLast(txt)   _HEXS(txt)
#define SpaceItHex(...)   ForEachMacro(_joinH, _joinHLast, __VA_ARGS__)

// -----------------------------------------------------
// create the _methods(...) that calls _method(arg) for each arg
#define _forEach(_method) _method##s

#define ForEachMethod(_method)                              \
inline void _forEach(_method)() {};                         \
template<class First, class... Args>                        \
inline void _forEach(_method)(First& first, Args&... args)  \
{                                                           \
  _method(first);                                           \
  _forEach(_method)(args...);                               \
};

// create the _methods(...) that calls _method(arg1, arg2) for each args' pair 
// _methods(...) needs an even number of args
#define ForEachMethodPairs(_method)                                  \
inline void _forEach(_method)() {};                                  \
template<class First, class Sec, class... Args>                      \
inline void _forEach(_method)(First& first, Sec &sec, Args&... args) \
{                                                                    \
  _method(first, sec);                                               \
  _forEach(_method)(args...);                                        \
};
