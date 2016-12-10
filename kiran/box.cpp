//==================================================================
// box.cpp   The box
//
// Author:   Vilas Kumar Chitrakaran (cvilas@ces.clemson.edu)
//           May 2004
//==================================================================


#include "objects.hpp"


//==================================================================
// class Box
//==================================================================
Box::Box(vector3d_t lo, vector3d_t hi, char *name)
{
 setVertices(lo, hi);
 setName(name);
}


Box::~Box()
{
}


void Box::setVertices(vector3d_t lo, vector3d_t hi)
{
 d_vl = lo;
 d_vh = hi;
}


rgb_t Box::getColor(vector3d_t pos) const
{
 return d_color;
}


intercept_t Box::getIntercept(const ray_t &ray) const
{
 intercept_t intercept;
 intercept.object = NULL;

 double tnear, tfar, t1, t2, tmp;
 
 tnear = -1e10;
 tfar = 1e10;
 
 // test for intersection with X planes
 if( (fabs(ray.dir.x) < 1e-6) )
 {
  if( (ray.orig.x < d_vl.x) || (ray.orig.x > d_vh.x) )
   return intercept;
 }
 else
 {
  t1 = (d_vl.x - ray.orig.x)/(ray.dir.x);
  t2 = (d_vh.x - ray.orig.x)/(ray.dir.x);
  
  if(t1 > t2)
  {
   tmp = t1;
   t1 = t2;
   t2 = tmp;
  }

  if(t1 > tnear) tnear = t1;
  if(t2 < tfar) tfar = t2;
  if( (tnear > tfar) || (tfar < 1e-6) ) return intercept;
 }

 // test for intersection with Y planes
 if( (fabs(ray.dir.y) < 1e-6) )
 {
  if( (ray.orig.y < d_vl.y) || (ray.orig.y > d_vh.y) )
   return intercept;
 }
 else
 {
  t1 = (d_vl.y - ray.orig.y)/(ray.dir.y);
  t2 = (d_vh.y - ray.orig.y)/(ray.dir.y);

  if(t1 > t2)
  {
   tmp = t1;
   t1 = t2;
   t2 = tmp;
  }

  if(t1 > tnear) tnear = t1;
  if(t2 < tfar) tfar = t2;
  if( (tnear > tfar) || (tfar < 1e-6) ) return intercept;
 }

 // test for intersection with Z planes
 if( (fabs(ray.dir.z) < 1e-6) )
 {
  if( (ray.orig.z < d_vl.z) || (ray.orig.z > d_vh.z) )
   return intercept;
 }
 else
 {
  t1 = (d_vl.z - ray.orig.z)/(ray.dir.z);
  t2 = (d_vh.z - ray.orig.z)/(ray.dir.z);
  
  if(t1 > t2)
  {
   tmp = t1;
   t1 = t2;
   t2 = tmp;
  }

  if(t1 > tnear) tnear = t1;
  if(t2 < tfar) tfar = t2;
  if( (tnear > tfar) || (tfar < 1e-6) ) return intercept;
 }
 
 intercept.coord = ray.orig + (ray.dir * tnear);
 intercept.incidentRay = ray.dir;
 intercept.object = (Object *)this;

 if( fabs(intercept.coord.z - d_vh.z) < 1e-6)
  intercept.normal = vector3d_t(0,0,1);
 else if( fabs(intercept.coord.z - d_vl.z) < 1e-6)
  intercept.normal = vector3d_t(0,0,-1);
 else if( fabs(intercept.coord.y - d_vh.y) < 1e-6)
  intercept.normal = vector3d_t(0,1,0);
 else if( fabs(intercept.coord.y - d_vl.y) < 1e-6)
  intercept.normal = vector3d_t(0,-1, 0);
 else if( fabs(intercept.coord.x - d_vh.x) < 1e-6)
  intercept.normal = vector3d_t(1,0,0);
 else if( fabs(intercept.coord.x - d_vl.x) < 1e-6)
  intercept.normal = vector3d_t(-1, 0, 0);

 return intercept;
}
