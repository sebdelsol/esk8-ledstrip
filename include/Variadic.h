#pragma once

// -----------------------------------------------------
// create a macro that calls a _m macro for each arg (max : see N in createVarMac.py)

#include <varMac.h> // run createVarMac.py to create all _Mn(_m, _p, x) macros
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

#define ForEachMethod(_method)                                  \
  inline void _forEach(_method)() {};                           \
  template<class First, class... Args>                          \
    inline void _forEach(_method)(First& first, Args&... args)  \
    {                                                           \
      _method(first);                                           \
      _forEach(_method)(args...);                               \
    };
