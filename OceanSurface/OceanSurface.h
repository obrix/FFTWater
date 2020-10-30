#ifndef OceanSurface_h
#define OceanSurface_h

#include <math.h>
#include <complex>
#include <vector>
#include "Vec2f.h"

namespace rx
{

class OceanSurface
{
public:

  OceanSurface();

  ~OceanSurface();

  void PrecomputeFields();

  void ComputeHeightmap(float t);

  typedef std::complex<float> ComplexN;

  struct InitialFields
  {
    float* waveVectorLength;
    float* waveFrequency;
    Vec2f* waveVector;

    // h~0(k) amplitude and phase computed with phillips spectrum
    ComplexN* h0_tilde;
    ComplexN* h0_tilde2;

    //field of frequency amplitude
    //h(k,t) = h~0(k) * exp(i*w(k)*t) + h~0(-k) * exp(-i*w(k)*t)
    ComplexN* h;
  };

  float ph_spectrum(Vec2f waveVector, float normWaveVector);
  Vec2f getWaveVector(unsigned int x, unsigned int z);
  ComplexN compute_h0tilde(Vec2f const& waveVector, float normWaveVector, float random_r, float random_i);
  ComplexN conjuguate(ComplexN in);

  //Discrete width and height
  unsigned m_dwidth;
  unsigned m_dheight;

  float m_hfieldSize; //cell size
  float m_windx;
  float m_windz;
  float m_waveheightfactor; // A in the formula
  float m_gravit_cst;
  float m_displace_lambda;

  float* m_heightmapData;
  float* m_displacementX;
  float* m_displacementZ;
  float* m_normalX;
  float* m_normalZ;

  InitialFields m_iFields;
};

}

#endif
