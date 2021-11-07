#ifndef OCEANSURFACEEXPORT_HXX
#define OCEANSURFACEEXPORT_HXX

#if defined (WIN32)
  #define OceanSurface_EXPORT __declspec(dllexport)
#else
  #define OceanSurface_EXPORT
#endif

#endif
