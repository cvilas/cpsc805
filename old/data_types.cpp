#include "data_types.hpp"
#include <math.h>

//==================================================================
// norm
//==================================================================
double norm( const vector3d_t &vector )
{
 return sqrt(vector.x * vector.x + 
             vector.y * vector.y + 
             vector.z * vector.z);
}


//==================================================================
// normalize
//==================================================================
vector3d_t normalize(const vector3d_t &vector)
{
 return (vector * (1/norm(vector)));
}


//==================================================================
// operator+ for vector3d_t
//==================================================================
vector3d_t operator+(const vector3d_t &v1, const vector3d_t &v2)
{
 vector3d_t result;
 result.x = v1.x + v2.x;
 result.y = v1.y + v2.y;
 result.z = v1.z + v2.z;
 return result;
}


//==================================================================
// operator- for vector3d_t
//==================================================================
vector3d_t operator-(const vector3d_t &v1, const vector3d_t &v2)
{
 vector3d_t result;
 result.x = v1.x - v2.x;
 result.y = v1.y - v2.y;
 result.z = v1.z - v2.z;
 return result;
}


//==================================================================
// operator* for vector3d_t
//==================================================================
vector3d_t operator*(const double &scalar, const vector3d_t &vector)
{
 vector3d_t result;
 result.x = vector.x * scalar;
 result.y = vector.y * scalar;
 result.z = vector.z * scalar;
 return result; 
}


vector3d_t operator*(const vector3d_t &vector, const double &scalar)
{
 vector3d_t result;
 result.x = vector.x * scalar;
 result.y = vector.y * scalar;
 result.z = vector.z * scalar;
 return result; 
}


//==================================================================
// dot product between two vector3d_t
//==================================================================
double dot(const vector3d_t &v1, const vector3d_t &v2)
{
 return (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
}


//==================================================================
// cross product between two vector3d_t
//==================================================================
vector3d_t cross(const vector3d_t &v1, const vector3d_t &v2)
{
 vector3d_t result;
 result.x = v1.y * v2.z - v2.y * v1.z;
 result.y = v2.x * v1.z - v1.x * v2.z;
 result.z = v1.x * v2.y - v2.x * v1.y;
 return result;
}


//==================================================================
// operator+ for rgb_t
//==================================================================
rgb_t operator+(const rgb_t &v1, const rgb_t &v2)
{
 rgb_t color;
 color.r = v1.r + v2.r;
 color.g = v1.g + v2.g;
 color.b = v1.b + v2.b;
 if(color.r > 1.0) color.r = 1.0;
 if(color.g > 1.0) color.g = 1.0;
 if(color.b > 1.0) color.b = 1.0;
 return color;
}


//==================================================================
// operator* for rgb_t
//==================================================================
rgb_t operator*(const double &scalar, const rgb_t &c)
{
 rgb_t color;
 double factor = fabs(scalar);
 color.r = c.r * factor;
 color.g = c.g * factor;
 color.b = c.b * factor;
 if(color.r > 1.0) color.r = 1.0;
 if(color.r > 1.0) color.r = 1.0;
 if(color.r > 1.0) color.r = 1.0;
 return color; 
}


rgb_t operator*(const rgb_t &c, const double &scalar)
{
 rgb_t color;
 double factor = fabs(scalar);
 color.r = c.r * factor;
 color.g = c.g * factor;
 color.b = c.b * factor;
 if(color.r > 1.0) color.r = 1.0;
 if(color.r > 1.0) color.r = 1.0;
 if(color.r > 1.0) color.r = 1.0;
 return color; 
}


//==================================================================
// operator* for transform_t
//==================================================================
vector3d_t operator*(const transform_t &t, const vector3d_t &v)
{
 vector3d_t r;
 r.x = t.t[0][0] * v.x + t.t[0][1] * v.y + t.t[0][2] * v.z + t.t[0][3];
 r.y = t.t[1][0] * v.x + t.t[1][1] * v.y + t.t[1][2] * v.z + t.t[1][3];
 r.z = t.t[2][0] * v.x + t.t[2][1] * v.y + t.t[2][2] * v.z + t.t[2][3];
 return r;
}


transform_t operator*(const transform_t &t1, const transform_t &t2)
{
 transform_t t;
 for(int i = 0; i < 4; i++)
  for(int j = 0; j < 4; j++)
  {
   t.t[i][j] = 0;
   for(int k = 0; k < 4; k++)
    t.t[i][j] += t1.t[i][k] + t2.t[k][j];
  }
 return t;
}


//==================================================================
// set_rotation
//==================================================================
transform_t set_rotation(double rx, double ry, double rz)
{
 transform_t t(0, 0, 0, rx, ry, rz);
 return t;
}


//==================================================================
// set_translation
//==================================================================
transform_t set_translation(double x, double y, double z)
{
 transform_t t(x, y, z, 0, 0, 0);
 return t;
}


//==================================================================
// set_rotation_about
//==================================================================
transform_t set_rotation_about(vector3d_t u, double theta)
{
 transform_t t;
 double ct, st;
 double vt;

 ct = cos(theta);
 st = sin(theta);
 vt = 1 - ct;
 
 t.t[0][0] = u.x * u.x * vt + ct;
 t.t[0][1] = u.x * u.y * vt - u.z * st;
 t.t[0][2] = u.x * u.z * vt + u.y * st;
 t.t[1][0] = u.x * u.y * vt + u.z * st;
 t.t[1][1] = u.y * u.y * vt + ct;
 t.t[1][2] = u.y * u.z * vt - u.x * st;
 t.t[2][0] = u.x * u.z * vt - u.y * st;
 t.t[2][1] = u.y * u.z * vt + u.x * st;
 t.t[2][2] = u.z * u.z * vt + ct;
 
 return t;
}


//==================================================================
// inverse
//==================================================================
transform_t inverse(transform_t t)
{
 transform_t it;
 vector3d_t c1(t.t[0][0], t.t[1][0], t.t[2][0]), 
            c2(t.t[0][1], t.t[1][1], t.t[2][1]), 
            c3(t.t[0][2], t.t[1][2], t.t[2][2]);
 vector3d_t p(t.t[0][3], t.t[1][3], t.t[2][3]);
 
 it.t[0][0] = t.t[0][0];
 it.t[0][1] = t.t[1][0];
 it.t[0][2] = t.t[2][0];

 it.t[1][0] = t.t[0][1];
 it.t[1][1] = t.t[1][1];
 it.t[1][2] = t.t[2][1];
 
 it.t[2][0] = t.t[0][2];
 it.t[2][1] = t.t[1][2];
 it.t[2][2] = t.t[2][2];
 
 it.t[0][3] = -dot(p, c1);
 it.t[1][3] = -dot(p, c2);
 it.t[2][3] = -dot(p, c3);

 it.t[3][3] = 1;
 
 return it;
}


//==================================================================
// get_rotation
//==================================================================
void get_rotation(const transform_t &t, double &rx, double &ry, double &rz)
{
 ry = -asin(t.t[2][0]);
 rx = asin((t.t[2][1])/cos(ry));
 rz = asin((t.t[1][0])/cos(ry));
}


//==================================================================
// get_translation
//==================================================================
vector3d_t get_translation(transform_t t)
{
 vector3d_t v;
 
 v.x = t.t[0][3];
 v.y = t.t[1][3];
 v.z = t.t[2][3];
 
 return v;
}


