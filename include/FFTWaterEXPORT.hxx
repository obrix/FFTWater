#ifndef FFTWATEREXPORT_HXX
#define FFTWATEREXPORT_HXX

#if defined (WIN32)
  #define FFTWater_EXPORT __declspec(dllexport)
#else
  #define FFTWater_EXPORT
#endif

#endif
