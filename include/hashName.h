#pragma once

#include <log.h>

// hash for [name] => Class* obj
// obj->name needs to be defined
template <int N, class Class>
class HashName 
{
  static constexpr int getN(uint8_t n) { return n * 2; }; // the bigger the less collisions

  Class*      objs[getN(N)];
  uint8_t     maxCol = 0;

  inline uint8_t hash(const char *name) // djb2: http://www.cse.yorku.ca/~oz/hash.html
  {
    unsigned long hash = 5381;
    int c;
    while (c = *name++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash % getN(N);;
  }

  // quadratic probing
  inline uint8_t next(uint8_t i, uint8_t col) { return (i + col * col) % getN(N); };

public:
  HashName() { memset( objs, 0, getN(N) ); };
  
  void add(Class* obj)
  {
    assert (obj!=nullptr);
    const char *name = obj->name;
    assert (name!=nullptr);
    
    uint8_t col = 0;
    uint8_t i = hash(name);
    
    // lookup for an empty slot
    while(objs[i] != nullptr) 
      i = next(i, ++col);

    maxCol = col > maxCol ? col : maxCol;
    if (col > 0) _log << " [" << name << "]: +" << col << " lookup" << (col > 1 ? "s" : "") << endl;
    objs[i] = obj;
  };

  inline bool objHasNotMyName(Class* obj, const char* name) 
  {
    const char* oname = obj->name;
    return oname != nullptr && strcmp(oname, name) != 0 ? true : false;
  };

  Class* get(const char *name)
  {
    assert (name!=nullptr);
    
    uint8_t col = 0;
    uint8_t i = hash(name);
    
    // lookup for an obj named name
    while(objs[i] != nullptr && objHasNotMyName(objs[i], name) ) 
    {
      if (++col > maxCol) return nullptr; // failed
      i = next(i, col);
    }
    return objs[i];
  };
};
