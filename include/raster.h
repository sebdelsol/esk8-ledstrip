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
    _log << "Free " << _FLOATW(ESP.getFreeHeap() / 1024., 4, 8) << " kB";
    _log << " - Loop "  << _WIDTH(end - lastEnd, 6) << "µs";                                                                    
    _log << " - rasters " << _WIDTH(end - start, 5) << "µs";                                                                  
    for (byte i=0; i < n; i++)
    {
      _log << "  - " << r[i].name << " " << _WIDTH(r[i].time - start, 5) << "µs"; 
      start = r[i].time;
    }
    _log << endl;
    lastEnd = end;
  };

#else
  inline void begin(){};       
  inline void add(const char*){};
  inline void end(){};
#endif
};
