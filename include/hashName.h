#pragma once
#include <Streaming.h>

// linear Probe hash for [name] => Class*
template <int N, class Class>
class HashName 
{
  static constexpr int getN(uint8_t n) { return n * 2; }; // bigger but less long collisions

  const char* _keys[getN(N)];
  Class*      _values[getN(N)];
  uint8_t     maxCol = 0;

  inline uint8_t _hash(const char *key) // naive hash, works ok with N*2
  {
    uint8_t h = 0;
    while (*key) h += *key++;
    return h % getN(N);
  };

  inline uint8_t _next(uint8_t i) { return (i+1) % getN(N); };

public:
  HashName() { memset( _values, NULL, getN(N) ); };
  
  void add(const char *key, Class* value)
  {
    assert (key!=nullptr);
    
    uint8_t col = 0;
    uint8_t i = _hash(key);
    while(_values[i] != nullptr)
    {
      i = _next(i);
      col++;
    }
    maxCol = col > maxCol ? col : maxCol;

    if (col > 0)
      Serial << ">> hash of [" << key << "] has " << col << " collisions" << endl;

    _values[i] = value;
    _keys[i] = key;
  };

  Class* get(const char *key)
  {
    assert (key!=nullptr);
    
    uint8_t col = 0;
    uint8_t i = _hash(key);
    while(_values[i] != nullptr && strcmp(key, _keys[i]) != 0 ) 
    {
      if (++col > maxCol) return nullptr; // failed
      i = _next(i);
    }
    
    // Serial << "got " << key << " stored@" << i << endl;
    return _values[i];
  };
};
