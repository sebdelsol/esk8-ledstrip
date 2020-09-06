# pragma once

// very basic iterator for static arrays
template <class T>
class iterArr 
{
  T* ptr;
public:
  iterArr(T* ptr) : ptr(ptr) {};
  
  iterArr operator++()                           { ++ptr; return *this; };     // increment
  bool    operator!=(const iterArr& other) const { return ptr != other.ptr; }; // compare
  T&      operator*()                      const { return *ptr; };             // deference
};

#define ArrayIterator(type, arr, max)                                           \
iterArr<type* const> begin() const { return iterArr<type* const>(arr); };       \
iterArr<type* const> end()   const { return iterArr<type* const>(arr + max); };
