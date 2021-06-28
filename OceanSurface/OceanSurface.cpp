#include "OceanSurface.h"
#include <fftw3.h>
#include <iostream>

namespace rx
{

OceanSurface::OceanSurface():
  m_dwidth(128),
  m_dheight(128),
  m_hfieldSize(100.0),
  m_windx(30.0),
  m_windz(0.0),
  m_waveheightfactor(0.0002),
  m_gravit_cst(9.81),
  m_displace_lambda(0.8)
{
  m_iFields.waveVectorLength = new float[m_dwidth * m_dheight];
  m_iFields.waveFrequency = new float[m_dwidth * m_dheight];
  m_iFields.h0_tilde = new ComplexN[m_dwidth * m_dheight];
  m_iFields.h0_tilde2 = new ComplexN[m_dwidth * m_dheight];
  m_iFields.waveVector = new Vec2f[m_dwidth * m_dheight];

  m_heightmapData = new float[m_dwidth * m_dheight];
  m_displacementX = new float[m_dwidth * m_dheight];
  m_displacementZ = new float[m_dwidth * m_dheight];

  m_normalX = new float[m_dwidth * m_dheight];
  m_normalZ = new float[m_dwidth * m_dheight];
}

OceanSurface::~OceanSurface()
{
  delete[] m_iFields.waveVectorLength;
  delete[] m_iFields.waveFrequency;
  delete[] m_iFields.h0_tilde;
  delete[] m_iFields.h0_tilde2;
  delete[] m_iFields.waveVector;

  delete[] m_heightmapData;
  delete[] m_displacementX;
  delete[] m_displacementZ;
}

void OceanSurface::PrecomputeFields()
{
  std::random_device rd{};
  std::mt19937 gen{rd()};
  std::normal_distribution<> d{0,1};

  for(unsigned i=0 ;i  < m_dwidth; i++)
  {
    for(unsigned j= 0; j < m_dheight; j++)
    {
      m_iFields.waveVector[m_dwidth*j + i] = getWaveVector(i,j);
      m_iFields.waveVectorLength[m_dwidth*j + i] = m_iFields.waveVector[m_dwidth*j + i].length();
      if(m_iFields.waveVectorLength[m_dwidth*j + i] < 0.1)
      {
        m_iFields.waveFrequency[m_dwidth*j + i] = 0.0;
      }
      else
      {
        m_iFields.waveFrequency[m_dwidth*j + i] = sqrt(m_gravit_cst * m_iFields.waveVectorLength[m_dwidth*j + i]);
      }
      float random_r = d(gen);
      float random_i = d(gen);
      m_iFields.h0_tilde[m_dwidth*j + i] = compute_h0tilde(m_iFields.waveVector[m_dwidth*j + i] ,m_iFields.waveVectorLength[m_dwidth*j + i],random_r,random_i);
      m_iFields.h0_tilde2[m_dwidth*j + i] = conjuguate(compute_h0tilde(-m_iFields.waveVector[m_dwidth*j + i] ,m_iFields.waveVectorLength[m_dwidth*j + i],random_r,random_i));
    }
  }
}

void OceanSurface::ComputeHeightmap(float t)
{
   //FFT

  //heightfield
  fftw_complex *in, *out;
  fftw_plan p;

  //displacement
  fftw_complex *inDx, *outDx;
  fftw_complex *inDz, *outDz;
  fftw_plan pDx;
  fftw_plan pDz;

  //normals
  fftw_complex *inNx, *outNx;
  fftw_complex *inNz, *outNz;
  fftw_plan pNx;
  fftw_plan pNz;

  //heightfield
  in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * m_dwidth * m_dheight);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * m_dwidth * m_dheight);

  //Displacement X for choppy waves
  inDx = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * m_dwidth * m_dheight);
    outDx = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * m_dwidth * m_dheight);

  //Displacecement Z for choppy waves
  inDz = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * m_dwidth * m_dheight);
    outDz = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * m_dwidth * m_dheight);

  //X normal
  inNx = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * m_dwidth * m_dheight);
    outNx = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * m_dwidth * m_dheight);

  //Z normal
  inNz = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * m_dwidth * m_dheight);
    outNz = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * m_dwidth * m_dheight);

  //Prepare the plan for fft

    p = fftw_plan_dft_2d(m_dwidth , m_dheight, in, out, FFTW_BACKWARD, FFTW_ESTIMATE);

  pDx = fftw_plan_dft_2d(m_dwidth , m_dheight, inDx, outDx, FFTW_BACKWARD, FFTW_ESTIMATE);

  pDz = fftw_plan_dft_2d(m_dwidth , m_dheight, inDz, outDz, FFTW_BACKWARD, FFTW_ESTIMATE);

  pNx = fftw_plan_dft_2d(m_dwidth , m_dheight, inNx, outNx, FFTW_BACKWARD, FFTW_ESTIMATE);

  pNz = fftw_plan_dft_2d(m_dwidth , m_dheight, inNz, outNz, FFTW_BACKWARD, FFTW_ESTIMATE);

  // in data for FFT
  for(int i = 0; i < m_dwidth * m_dheight; i++)
  {
    ComplexN e1(cos(m_iFields.waveFrequency[i]*t),sin(m_iFields.waveFrequency[i]*t));
    ComplexN e2 = conjuguate(e1);
    float k_dot_x = Vec2f::dot(m_iFields.waveVector[i].normalized(), Vec2f(i%m_dwidth,i/m_dwidth).normalized());
    ComplexN h_tilde = (m_iFields.h0_tilde[i] * e1 + m_iFields.h0_tilde2[i] * e2)*k_dot_x;

    ComplexN h_tiled_slopex = h_tilde* ComplexN(0.0,m_iFields.waveVector[i].x);
    ComplexN h_tiled_slopez = h_tilde* ComplexN(0.0,m_iFields.waveVector[i].y);


    ComplexN h_tiled_displacex;
    ComplexN h_tiled_displacez;

    if(m_iFields.waveVectorLength[i] >0.0001)
    {
      h_tiled_displacex = h_tilde* ComplexN(0.0,-m_iFields.waveVector[i].x / m_iFields.waveVectorLength[i]);
      h_tiled_displacez = h_tilde* ComplexN(0.0,-m_iFields.waveVector[i].y / m_iFields.waveVectorLength[i]);
    }
    else
    {
      h_tiled_displacex = h_tilde* ComplexN(0.0,0.0);
      h_tiled_displacez = h_tilde* ComplexN(0.0,0.0);
    }

    in[i][0] = h_tilde.real();
    in[i][1] = h_tilde.imag();

    inDx[i][0] = h_tiled_displacex.real();
    inDx[i][1] = h_tiled_displacez.imag();

    inDz[i][0] = h_tiled_displacex.real();
    inDz[i][1] = h_tiled_displacez.imag();

    inNx[i][0] = h_tiled_slopex.real();
    inNx[i][1] = h_tiled_slopex.imag();

    inNz[i][0] = h_tiled_slopez.real();
    inNz[i][1] = h_tiled_slopez.imag();
  }

  //FFTs!!
  fftw_execute(p);
  fftw_execute(pDx);
  fftw_execute(pDz);
  fftw_execute(pNx);
  fftw_execute(pNz);

  int signs[] = {1, -1};

  //out data from FFT
  for(int i = 0; i < m_dwidth * m_dheight; i++)
  {
    int x = i% m_dwidth;
    int z = i/m_dheight;
    int sign = signs[(x + z) & 1];

    m_heightmapData[i] =  sign* out[i][0];
    m_displacementX[i] = sign*m_displace_lambda*outDx[i][0];
    m_displacementZ[i] = sign*m_displace_lambda*outDz[i][0];
    m_normalX[i] = sign*outNx[i][0];
    m_normalZ[i] = sign*outNz[i][0];
  }

  //Destroy the plans
  fftw_destroy_plan(p);
  fftw_free(in);
  fftw_free(out);

  fftw_destroy_plan(pDx);
  fftw_free(inDx);
  fftw_free(outDx);

  fftw_destroy_plan(pDz);
  fftw_free(inDz);
  fftw_free(outDz);

  fftw_destroy_plan(pNx);
  fftw_free(inNx);
  fftw_free(outNx);

  fftw_destroy_plan(pNz);
  fftw_free(inNz);
  fftw_free(outNz);
}

OceanSurface::ComplexN OceanSurface::compute_h0tilde(Vec2f const& waveVector, float normWaveVector, float random_r, float random_i)
{
  float real = (1.0 / sqrt(2.0) ) * random_r * sqrt(ph_spectrum(waveVector,normWaveVector));
  float img = (1.0 / sqrt(2.0) ) * random_i * sqrt(ph_spectrum(waveVector,normWaveVector));

  return ComplexN(real,img);
}

//conjuguate of a complex number
OceanSurface::ComplexN OceanSurface::conjuguate(ComplexN in)
{
  return ComplexN(in.real(),-in.imag());
}

float OceanSurface::ph_spectrum(Vec2f waveVector, float normWaveVector)
{
  Vec2f windVector(m_windx, m_windz);
  float windSpeed = windVector.length();
  windVector.normalize();
  waveVector.normalize();

  if(normWaveVector < 0.1)
    return 0;
  float L = (windSpeed * windSpeed) / m_gravit_cst; // L
  float e = exp(-1.0 / pow(normWaveVector*L,2)); // exp part
  float k4 = normWaveVector*normWaveVector*normWaveVector*normWaveVector;
  float k_dot_w = pow(Vec2f::dot(waveVector, windVector), 2);

  float l = m_hfieldSize/100.0;

  return m_waveheightfactor * ( e / k4) * k_dot_w /*exp(-normWaveVector*normWaveVector*l*l)*/;
}

Vec2f OceanSurface::getWaveVector(unsigned int x, unsigned int z)
{
  int m,n;
  m = (int)x - (m_dwidth/2.0);
  n = (int)z - (m_dheight/2.0);
  float lx = m_hfieldSize;
  float lz = m_hfieldSize;

  return Vec2f( (2*M_PI * m) / lx, (2*M_PI * n) / lz );
}

}
