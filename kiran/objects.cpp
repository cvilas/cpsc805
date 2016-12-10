#include "objects.hpp"
#include <math.h>


//==================================================================
// class Object
//==================================================================

Object::Object()
{
 d_name="unknown object";
 d_hasTextureMap = false;
 d_hasBumpMap = false;
 d_texture = NULL;
 d_bumpMap = NULL;
 d_bumpiness = 0.01;
 d_kTrans = 0;
 d_kRef = 0;
 d_refrInd = 0;
 d_ka = 0.0;
 d_kd = 0.0;
 d_ks = 0.0;
 d_specRefExp = 1;
}


Object::~Object()
{
 if (d_texture)
  delete d_texture;
  
 if(d_bumpMap)
  delete d_bumpMap;
}


void Object::translate(double x, double y, double z)
{
 vector3d_t inBase;
 transform_t pos;
 inBase.x = d_transform.t[0][0] * x +
            d_transform.t[0][1] * y +
            d_transform.t[0][2] * z;
 inBase.y = d_transform.t[1][0] * x +
            d_transform.t[1][1] * y +
            d_transform.t[1][2] * z;
 inBase.z = d_transform.t[2][0] * x +
            d_transform.t[2][1] * y +
            d_transform.t[2][2] * z;
 d_transform = set_translation(inBase.x, inBase.y, inBase.z) * d_transform;
 d_iTransform = inverse(d_transform);
}

// programmer note: d_transform is defined such that if 
// Vl is defined relative to local coordinates, then Vg, 
// the vector in global coordinates is Vg  = d_transform * Vl
void Object::rotate(double x, double y, double z)
{
 vector3d_t inBase;
 inBase.x = d_transform.t[0][0] * x +
            d_transform.t[0][1] * y +
            d_transform.t[0][2] * z;
 inBase.y = d_transform.t[1][0] * x +
            d_transform.t[1][1] * y +
            d_transform.t[1][2] * z;
 inBase.z = d_transform.t[2][0] * x +
            d_transform.t[2][1] * y +
            d_transform.t[2][2] * z;
 d_transform = d_transform * set_rotation(inBase.x, inBase.y, inBase.z);
 d_iTransform = inverse(d_transform);
}


void Object::setTexture(char *ppmFileName)
{
 d_texture = new Pixmap(ppmFileName);
 d_hasTextureMap = true;
}


void Object::setBumpMap(char *ppmFileName)
{
 d_bumpMap = new Pixmap(ppmFileName);
 d_hasBumpMap = true;
}


//==================================================================
// class ZCylinder
//==================================================================
ZCylinder::ZCylinder(vector3d_t c, double r, double e, char *name)
{
 setRadius(r);
 setPosition(c);
 setLength(e);
 setName(name);
 d_hasEndCaps = false;
}


rgb_t ZCylinder::getColor(vector3d_t pos) const
{
 return d_color;
}


intercept_t ZCylinder::getIntercept(const ray_t &ray) const
{
 intercept_t intercept;
 intercept.object = NULL;
 intercept.incidentRay = ray.dir;
 
 double a, b, c, d, dsq, t1, t2, te, ts, zmin, zmax;
 vector3d_t ve;

 te = ts = 1e10;
 zmin = d_center.z;
 zmax = zmin + d_length;
 
 // Check endcap intercepts
 t1 = (zmin - ray.orig.z)/ray.dir.z;
 d = pow((ray.orig.x - d_center.x + ray.dir.x * t1), 2 ) 
      + pow((ray.orig.y - d_center.y + ray.dir.y * t1), 2) 
      - d_radius * d_radius;
 
 if(t1 > 0 && d < 0)
  te = t1;
 
 t2 = (zmax - ray.orig.z)/ray.dir.z;
 d = pow((ray.orig.x - d_center.x + ray.dir.x * t2), 2 ) 
      + pow((ray.orig.y - d_center.y + ray.dir.y * t2), 2) 
      - d_radius * d_radius;

 if(t2 > 0 && d < 0 && t2 < te)
  te = t2;

 if(t1 < 1e-5 && t2 < 1e-5) // object behind eye
  return intercept;
 
 // computations for surface intercept follows
 a = ray.dir.x * ray.dir.x + ray.dir.y * ray.dir.y;
 b = -2 * ((d_center.x - ray.orig.x)*ray.dir.x + (d_center.y - ray.orig.y)*ray.dir.y);
 c = (d_center.x - ray.orig.x) * (d_center.x - ray.orig.x) + 
     (d_center.y - ray.orig.y) * (d_center.y - ray.orig.y) - d_radius * d_radius;
 dsq = b * b - 4 * a * c;

 if(dsq < 0 ) // no surface intercept. end caps?
 {
  if(te < 1e10 && d_hasEndCaps) // if we have a valid te
  {
   ve = ray.orig + ray.dir * te;
   intercept.coord = ve;
   if(fabs(ve.z - zmin) < 1e-5)
    intercept.normal = vector3d_t(0,0, -1);
   else
    intercept.normal = vector3d_t(0,0, 1);
   intercept.normal = normalize(intercept.normal);
   intercept.object = (Object *)this;
  }
  return intercept;
 }

 t1 = (-b + sqrt(dsq))/(2 * a); 
 t2 = (-b - sqrt(dsq))/(2 * a); 

 if(t1 < 1e-5 && t2 < 1e-5)
  return intercept;
  
 d = ray.orig.z + ray.dir.z * t1;
 
 if(t1 > 0 && d > zmin && d < zmax)
  ts = t1;
 
 d = ray.orig.z + ray.dir.z * t2;
 
 if(t1 > 0 && d > zmin && d < zmax && t2 < ts)
  ts = t2;

 if(ts == 1e10 && te == 1e10)
  return intercept; // no valid intercepts


 if(te < ts && d_hasEndCaps)
 {
  ve = ray.orig + ray.dir * te;
  intercept.coord = ve;
   if(fabs(ve.z - zmin) < 1e-5)
    intercept.normal = vector3d_t(0,0, -1);
   else
    intercept.normal = vector3d_t(0,0, 1);
 }
 else
 {
  intercept.coord = ray.orig + ray.dir * ts;
  intercept.normal.x = intercept.coord.x - d_center.x;
  intercept.normal.y = intercept.coord.y - d_center.y;
  intercept.normal.z = 0;
 }
 intercept.normal = normalize(intercept.normal);
 intercept.object = (Object *)this;

 return intercept;
}


void ZCylinder::setEndCapsOn()
{
 d_hasEndCaps = true;
}


//==================================================================
// findLineLineIntercept 
//==================================================================
vector3d_t findLineLineIntercept(const vector3d_t &v1, const vector3d_t &v2,
                                 const vector3d_t &v3, const vector3d_t &v4)
{
 vector3d_t a, b, c, d, i;
 double t, l;
 
 a = v2 - v1;
 b = v4 - v3;
 c = v3 - v1;
 d = cross(a,b);
 l= norm(d);
 
 if(l != 0.0)
 {
  t = dot(cross(c, b), d)/(l*l);
  i = v1 + (a * t);
 }
 
 return i;
}


//==================================================================
// findTriangleCog
//==================================================================
vector3d_t findTriangleCog(const vector3d_t &v1, const vector3d_t &v2, 
                           const vector3d_t &v3)
{
 vector3d_t i;
 vector3d_t v23; // center of v2->v3
 vector3d_t v13; // center of v1->v3
 
 v23 = 0.5 * (v2 + v3);
 v13 = 0.5 * (v1 + v3);
 
 //CoG is the intersection of vectors v1->v23 and v2->v13 
 i = findLineLineIntercept(v1, v23, v2, v13);

 return i;
}


