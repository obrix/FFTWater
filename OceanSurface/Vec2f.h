#ifndef Vec2f_h
#define Vec2f_h

#include "FFTWaterEXPORT.hxx"
//Minimalist Vec2f

struct FFTWater_EXPORT Vec2f
{
  Vec2f();

  Vec2f(Vec2f const& p_o);

  Vec2f(float px, float py);

  //Return the norm of the vector
  float length();

  //Normalize the vector
  void normalize();

  Vec2f normalized();

  //Return the dot product of two vectors
  static float dot(Vec2f const& p_a, Vec2f const& p_b);

  Vec2f operator-();

  float x;
  float y;
};

#endif
