//==================================================================
// data_types.hpp  Commonly used data types
//
// Author:         Vilas Kumar Chitrakaran (cvilas@ces.clemson.edu)
//                 Feb 2004
//==================================================================


#ifndef _DATA_TYPES_HPP_INCLUDED
#define _DATA_TYPES_HPP_INCLUDED

#include <iostream>
#include <string>
#include <math.h>

using namespace std;

//==================================================================
// struct _rgb  A simple structure to represent colors.
//==================================================================
typedef struct _rgb
{
 _rgb(double R = 0, double G = 0, double B = 0) : r(R), g(G), b(B) {};
 double r;
 double g;
 double b;
}rgb_t;


//==================================================================
// struct _vector3d  A simple structure for 3D vectors.
//==================================================================
typedef struct _vector3d
{
 _vector3d(double X = 0, double Y = 0, double Z = 0) : x(X), y(Y), z(Z) {};
 double x;
 double y;
 double z;
}vector3d_t;


//==================================================================
// struct _transform  Homogeneous transformation matrix
//==================================================================
typedef struct _transform
{
 double t[4][4];
 _transform(double x = 0, double y = 0, double z = 0, double rx = 0, 
            double ry = 0, double rz = 0) 
 {
  double cx, cy, cz, sx, sy, sz;
  cx = cos(rx); cy = cos(ry); cz = cos(rz);
  sx = sin(rx); sy = sin(ry); sz = sin(rz);
  
  this->t[0][3] = x;
  this->t[1][3] = y;
  this->t[2][3] = z;
  this->t[3][3] = 1;
  
  this->t[0][0] = cz * cy;
  this->t[0][1] = cz * sy * sx - sz * cx;
  this->t[0][2] = sz * sx + cz * sy * cx;

  this->t[1][0] = sz * cy;
  this->t[1][1] = sz * sy * sx + cz * cx;
  this->t[1][2] = -cz * sx + sz * sy * cx;

  this->t[2][0] = -sy;
  this->t[2][1] = cy * sx;
  this->t[2][2] = cy * cx;
  
  this->t[3][0] = this->t[3][1] = this->t[3][2] = 0;
 }
}transform_t;


//==================================================================
// struct _ray  A simple structure for a ray of light.
//==================================================================
typedef struct _ray
{
 vector3d_t orig;  // ray origin
 vector3d_t dir;   // ray direction
}ray_t;


//==================================================================
// struct _ray_list  A linked list of light rays.
//==================================================================
typedef struct _ray_list
{
 _ray_list(_ray_list *n = NULL)
          : nxt(n) {};
 _ray_list *nxt;
 ray_t ray;
}ray_list_t;

//==================================================================
// struct _intercept  A structure to hold interception data 
//                    between a ray and an object in the scene.
//==================================================================
class Object;
typedef struct _intercept
{
 vector3d_t coord;          // coordinates of intersection
 vector3d_t normal;         // surface normal
 vector3d_t incidentRay;    // direction from eye-point to intercept
 Object *object;            // object intercepted by ray
}intercept_t;


//==================================================================
// Operators for _vector3d
//==================================================================
double norm(const vector3d_t &vector);
vector3d_t normalize( const vector3d_t &vector);
vector3d_t operator+(const vector3d_t &v1, const vector3d_t &v2);
vector3d_t operator-(const vector3d_t &v1, const vector3d_t &v2);
vector3d_t operator*(const double &scalar, const vector3d_t &vector);
vector3d_t operator*(const vector3d_t &vector, const double &scalar);
double dot(const vector3d_t &v1, const vector3d_t &v2);
vector3d_t cross(const vector3d_t &v1, const vector3d_t &v2);


//==================================================================
// Operators for _rgb
//==================================================================
rgb_t operator+(const rgb_t &c1, const rgb_t &c2);
rgb_t operator*(const double &scalar, const rgb_t &c);
rgb_t operator*(const rgb_t &c, const double &scalar);


//==================================================================
// Operators for _transform
//==================================================================
vector3d_t operator*(const transform_t &t, const vector3d_t &v);
transform_t operator*(const transform_t &t1, const transform_t &t2);
transform_t set_translation(double x, double y, double z);
transform_t set_rotation(double rx, double ry, double rz);
transform_t set_rotation_about(vector3d_t axis, double angle);
void get_rotation(const transform_t &t, double &rx, double &ry, double &rz);
vector3d_t get_translation(transform_t t);
transform_t inverse(transform_t t);


#endif // ifndef _DATA_TYPES_HPP_INCLUDED
