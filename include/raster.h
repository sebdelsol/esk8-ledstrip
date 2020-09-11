#pragma once

struct Raster
{
#ifdef DEBUG_RASTER
  static const int max = 20;

  struct                                                                                                               
  {                                                                                                                           
    const char* name;                                                                                          
    long        time;                                                                                                                
  } r[max];                                                                                                     

  int  n;                                                                                                       
  long start;                                                                                                 
  long lastEnd;                                     

  inline void begin()
  {
    n = 0;                                                                                                           
    start = micros();
  };       

  // to be called only with string literal with static storage !!  
  inline void add(const char* name)
  {
    if (n < max)                                                                                              
    {                                                                                                                           
      r[n].time = micros();                                                                                   
      r[n++].name = name;                                                                                   
    }                                                                                                                           
    else                                                                                                                        
      _log << ">> ERROR !! Max Raster reached "  << n << endl;
  };

  inline void end()
  {
    long end = micros();                                                                                                   
    _log << "LOOP "  << (end - lastEnd) << "µs";                                                                    
    lastEnd = end;

    _log << " \t TOTAL " << (end - start) << "µs";                                                                  
    for (byte i=0; i < n; i++)                                                                                        
      _log << " \t - " << r[i].name << " " << (r[i].time - (i==0 ? start : r[i-1].time)) << "µs  "; 
    
    _log << "\t free Heap - " << ESP.getFreeHeap() << endl;
  };

#else
  inline void begin(){};       
  inline void add(const char*){};
  inline void end(){};
#endif
};
