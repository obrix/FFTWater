#ifndef OceanSurface_h
#define OceanSurface_h

#include <math.h>
#include <complex>
#include "Vec2f.h"
#include <vector>

class OceanSurface
{
public:

  OceanSurface();

  ~OceanSurface();

  void PrecomputeFields(std::vector<float>& randr, std::vector<float>& randi);

  void ComputeHeightmap(float t);

  typedef std::complex<float> Complex;

  struct InitialFields
  {
    float* waveVectorLength;
    float* waveFrequency;
    Vec2f* waveVector;

    // h~0(k) amplitude and phase computed with phillips spectrum
    Complex* h0_tilde;
    Complex* h0_tilde2;

    //field of frequency amplitude
    //h(k,t) = h~0(k) * exp(i*w(k)*t) + h~0(-k) * exp(-i*w(k)*t)
    Complex* h;
  };

  float ph_spectrum(Vec2f waveVector, float normWaveVector);
  Vec2f getWaveVector(unsigned int x, unsigned int z);
  Complex compute_h0tilde(Vec2f const& waveVector, float normWaveVector, float random_r, float random_i);
  Complex conjuguate(Complex in);

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

#endif
