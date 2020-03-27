//Minimalist Vec2f
#include "Vec2f.h"
#include <math.h>

Vec2f::Vec2f()
{
}

Vec2f::Vec2f(Vec2f const& p_o)
{
  this->x = p_o.x;
  this->y = p_o.y;
}

Vec2f::Vec2f(float px, float py):
  x(px),
  y(py)
{
}

float Vec2f::length()
{
  return sqrt(x*x + y*y);
};

void Vec2f::normalize()
{
  float l = length();
  if(l > 0)
  {
    x /= l;
    y /= l;
  }
  else
  {
    x = 0;
    y = 0;
  }
};

Vec2f Vec2f::normalized()
{
  Vec2f r;
  float l = length();
  if(l > 0)
  {
    r.x = x / l;
    r.y = y / l;
  }
  else
  {
    r.x = 0;
    r.y = 0;
  }

  return r;
}

float Vec2f::dot(Vec2f const& p_a, Vec2f const& p_b)
{
  return p_a.x * p_b.x + p_a.y * p_b.y;
};

Vec2f Vec2f::operator-()
{
  Vec2f r;
  r.x = -x;
  r.y = -y;
  return r;
}

