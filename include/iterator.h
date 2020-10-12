# pragma once

// very basic iterator for static arrays
template <class T>
class iterArr 
{
  T* ptr;
public:
  iterArr(T* ptr) : ptr(ptr) {};                                               // construct
  iterArr operator++()                           { ++ptr; return *this; };     // increment
  bool    operator!=(const iterArr& other) const { return ptr != other.ptr; }; // compare
  T&      operator*()                      const { return *ptr; };             // deference
};

// iterator for type* arr[] with a variable max
#define ArrayOfPtr_Iter(type, arr, max)                 \
using iterator = iterArr<type* const>;                  \
iterator begin() const { return iterator(arr); };       \
iterator end()   const { return iterator(arr + max); };
