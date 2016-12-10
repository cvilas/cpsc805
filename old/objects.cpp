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
 transform_t t = set_translation(x, y, z);
 d_transform = d_transform * t;
 d_iTransform = inverse(d_transform);
}

// programmer note: d_transform is defined such that if 
// Vl is defined relative to local coordinates, then Vg, 
// the vector in global coordinates is Vg  = d_transform * Vl
void Object::rotate(double x, double y, double z)
{
 transform_t t = set_rotation(x, y, z);
 d_transform = d_transform * t;
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
// class Sphere
//==================================================================

Sphere::Sphere(vector3d_t center, double radius, char *name)
{
 d_radius = radius;
 d_center = center;
 d_name = "Sphere::";
 d_name += name; 
 d_ka = 0.0001;
 d_kd = 0.001;
 d_ks = 0.01;
 d_specRefExp = 1;
}


Sphere::~Sphere()
{
}


void Sphere::setName(char *name)
{
 d_name = "Sphere::";
 d_name += name; 
}


rgb_t Sphere::getColor(vector3d_t pos) const
{
 rgb_t color;
 double u, v;
 if(!d_hasTextureMap)
  return d_color;

 doInverseSphereMap(pos, u, v);
 color = (*d_texture)((int)(u * (d_texture->width()-1) + 1), 
                      (int)((1-v) * (d_texture->height()-1))+1);
 return color;
}


intercept_t Sphere::getIntercept(const ray_t &ray) const
{
 intercept_t intercept;
 intercept.object = NULL;
 
 double a, b, c, dsq, t1, t2, t;
 vector3d_t eyeToCenter;
 
 eyeToCenter = ray.orig - d_center;
 
 a = pow( norm(ray.dir), 2);
 b = 2 * (dot(ray.dir, eyeToCenter));
 c = pow( norm(eyeToCenter), 2) - pow(d_radius, 2);

 dsq = b * b - 4 * a * c;
 if(dsq < 0 )
  return intercept;
 
 t1 = (-b + sqrt(dsq))/(2 * a); 
 t2 = (-b - sqrt(dsq))/(2 * a); 

 if( t1 < 0.0001 && t2 < 0.0001) // both intercepts negative or small
  return intercept;

 intercept.object = (Object *)this;
 intercept.incidentRay = ray.dir;
 
 if( t1 > 0.0001 && t2 > 0.0001 ) // sphere in front of eye
 {
  (t1 < t2) ? (t = t1) : (t = t2);
  intercept.coord = ray.orig + ray.dir * t;
  intercept.normal = intercept.coord - d_center;
  intercept.normal = normalize(intercept.normal);
  if(d_hasBumpMap)
   intercept.normal = getBumpedNormal(intercept);
  return intercept;
 }
 
 (t1 > 0.0001 ) ? (t = t1) : (t = t2); // eye inside sphere
 intercept.coord = ray.orig + ray.dir * t;
 intercept.normal = intercept.coord - d_center;
 intercept.normal = normalize(intercept.normal);
 if(d_hasBumpMap)
  intercept.normal = getBumpedNormal(intercept);
 return intercept;
}


void Sphere::doInverseSphereMap(const vector3d_t pos, double &u, double &v) const
{
 vector3d_t sn, sp, se;
 double phi, theta = 0, ut;
 
 sn = normalize(pos - d_center); // surface normal at pos
 sp = vector3d_t(0,-1,0); // north pole
 se = vector3d_t(1,0,0); // equator (using global coords for both here)
 sp = normalize(sp);
 se = normalize(se);
 
 phi = acos(-dot(sp, sn));
 v = phi/M_PI;
 
 if( v < 0.001 || v > 0.999)
  u = 0;
 else
 {
  double arg = 0;
  arg = dot(sn, se)/sin(phi);
  if(arg > 1.0)
   arg = 1.0;
  if(arg < -1.0)
   arg = -1.0;
   
  theta = acos(arg);
  ut = theta/(2 * M_PI); // temp
  if(dot(cross(sp, se), sn) > 0)
   u = ut;
  else
   u = 1 - ut;
 }
 
 if(u > 1.0) u = 1.0;
 if(u < 0) u = 0;
 if(v > 1.0) v = 1.0;
 if(v < 0) v = 0;
}


vector3d_t Sphere::doSphereMap(double u, double v) const
{
 vector3d_t vector;
 double phi, theta; // angles from Z and X respectively
 
 // assuming u and v are in range [0.0 1.0]
 phi = (1-v) * M_PI;
 theta = u * 2 * M_PI;
 
 vector.x = d_radius * sin(phi) * cos(theta);
 vector.y = d_radius * sin(phi) * sin(theta);
 vector.z = d_radius * cos(phi);
 vector = vector + d_center;
 return vector;
}


vector3d_t Sphere::getBumpedNormal(const intercept_t &intercept) const
{
 if(!d_hasBumpMap)
  return intercept.normal;
  
 vector3d_t newNormal, pos, pu, pd, pl, pr;
 double u, v, ut, vt;
 rgb_t bump;
 double bumpiness;
 
 pos = intercept.coord;
 doInverseSphereMap(pos, u, v); // get u, v for a intersection

 // create 4 new normals depending respectively on 
 // pixels values up, down, left and right of this pixel
 // in the bump map.
 ut = u-2.0/d_bumpMap->width();
 if(ut < 0.0) ut = 0.0;
 bump = (*d_bumpMap)((int)(ut * (d_bumpMap->width()-1) + 1), 
                      (int)((1-v) * (d_bumpMap->height()-1))+1);
 bumpiness = d_bumpiness * (0.33 * (bump.r + bump.g + bump.b) - 0.5);
 pl = doSphereMap(ut, v);
 pl = bumpiness * normalize(pl - d_center);

 ut = u+2.0/d_bumpMap->width();
 if(ut > 1.0) ut = 1.0;
 bump = (*d_bumpMap)((int)(ut * (d_bumpMap->width()-1) + 1), 
                      (int)((1-v) * (d_bumpMap->height()-1))+1);
 bumpiness = d_bumpiness * (0.33 * (bump.r + bump.g + bump.b) - 0.5);
 pr = doSphereMap(ut, v);
 pr = bumpiness * normalize(pr - d_center);
 
 vt = v+2.0/d_bumpMap->height();
 if(vt > 1.0) vt = 1.0;
 bump = (*d_bumpMap)((int)(u * (d_bumpMap->width()-1) + 1), 
                      (int)((1-vt) * (d_bumpMap->height()-1))+1);
 bumpiness = d_bumpiness * (0.33 * (bump.r + bump.g + bump.b) - 0.5);
 pu = doSphereMap(u, vt);
 pu = bumpiness * normalize(pu - d_center);
 
 vt = v-2.0/d_bumpMap->height();
 if(vt < 0.0) vt = 0.0;
 bump = (*d_bumpMap)((int)(u * (d_bumpMap->width()-1) + 1), 
                      (int)((1-vt) * (d_bumpMap->height()-1))+1);
 bumpiness = d_bumpiness * (0.33 * (bump.r + bump.g + bump.b) - 0.5);
 pd = doSphereMap(u, vt);
 pd = bumpiness * normalize(pd - d_center);

 newNormal = (pl + pr + pu + pd);
 newNormal = normalize(intercept.normal + newNormal);
 
 return newNormal;
}


//==================================================================
// class InfinitePlane
//==================================================================

InfinitePlane::InfinitePlane(vector3d_t normal, double distance, char *name)
{
 d_normal = normal;
 d_distance = distance;
 d_name = "Plane::";
 d_name += name; 
}


void InfinitePlane::setPosition(vector3d_t normal, double d)
{
 d_normal = normal;
 d_distance = d;
}


void InfinitePlane::setName(char *name)
{
 d_name = "Plane::";
 d_name += name; 
}


rgb_t InfinitePlane::getColor(vector3d_t pos) const
{
 return d_color;
}


intercept_t InfinitePlane::getIntercept(const ray_t &ray) const
{
 intercept_t intercept;
 double t;
 double nDotR;
 double nDotE;
 
 intercept.object = NULL;
 nDotR = dot(d_normal, ray.dir);
 nDotE = dot(d_normal, ray.orig);
 
 if( fabsf(nDotR) <= 1e-5 ) // ray parallel to plane
  return intercept;
 
 t = (-1.0) * (nDotE + d_distance) / (nDotR);
 
 if( t < 0.0001 ) // intercept behind eye
  return intercept;
 
 intercept.object = (Object *)this;
 intercept.coord = ray.orig + ray.dir * t;
 intercept.incidentRay = ray.dir;
 
 if( nDotR > 0 )
  intercept.normal = d_normal * (-1.0);
 else
  intercept.normal = d_normal;

 return intercept;
}


//==================================================================
// class CheckerBoard
//==================================================================

rgb_t CheckerBoard::getColor(vector3d_t pos) const
{
 double color;
 color = floorf(d_checkerScale * pos.x) + 
         floorf(d_checkerScale * pos.y) + 
         floorf(d_checkerScale * pos.z);

 if((int)color & 0x1) 
  return d_color2;

 return d_color;
}


//==================================================================
// class PlanarConvexQuad
//==================================================================
PlanarConvexQuad::PlanarConvexQuad(vector3d_t v1, vector3d_t v2, 
                                   vector3d_t v3, vector3d_t v4, 
                                   char *name)
{
 setVertices(v1, v2, v3, v4);
 setName(name);
}


void PlanarConvexQuad::setName(char *name)
{
 d_name = "PlanarConvexQuad::";
 d_name += name; 
}


void PlanarConvexQuad::setVertices(vector3d_t v1, vector3d_t v2, vector3d_t v3, vector3d_t v4)
{
 d_v1 = v1;
 d_v2 = v2;
 d_v3 = v3;
 d_v4 = v4;
 d_normal = cross((v2 - v1), (v4 - v1));
 d_normal = normalize(d_normal);
 d_distance = -1 * dot(d_normal, v1);
}


rgb_t PlanarConvexQuad::getColor(vector3d_t pos) const
{
 rgb_t color;
 double u, v;
 if(!d_hasTextureMap)
  return d_color;

 doInverseConvQuadMap(pos, u, v);
 color = (*d_texture)((int)(u * (d_texture->width()-1) + 1), 
                      (int)((1-v) * (d_texture->height()-1))+1);
 return color;
}


void PlanarConvexQuad::doInverseConvQuadMap(const vector3d_t pos, 
                             double &u, double &v) const
{
 vector3d_t pa, pb, pc, pd, na, nb, nc, qux, quy, qvx, qvy;
 double du0, du1, du2, dux, duy, dv0, dv1, dv2, dvx, dvy;
 double ka, kb, u0, u1, v0, v1, tmp;
 pd = d_v2;
 pc = d_v1 - d_v2;
 pb = d_v3 - d_v2;
 pa = d_v2 - d_v3 + d_v4 - d_v1;
 
 na = cross(pa, d_normal);
 nb = cross(pb, d_normal);
 nc = cross(pc, d_normal);
 
 du0 = dot(nc, pd);
 du1 = dot(na, pd) + dot(nc, pb);
 du2 = dot(na, pb);
 
 // find u
 if( fabs(du2) <= 1e-5 ) // 0 actually
 {
  u = (dot(pos, nc) - du0)/(du1 - dot(pos, na));
 }
 else
 {
  qux = (1.0/(2 * du2)) * na;
  dux = -du1 / (2 * du2);
  quy = -(1.0/du2) * nc;
  duy = du0/du2;
  
  ka = dux + dot(qux, pos);
  kb = duy + dot(quy, pos); 
  
  tmp = sqrt(ka * ka - kb);
  u0 = ka - tmp;
  u1 = ka + tmp;
  
  if( u0 <= 1.0 && u0 >= 0)
   u = u0;
  else
   u = u1;
 }

 // find v
 dv0 = dot(nb, pd);
 dv1 = dot(na, pd) + dot(nb, pc);
 dv2 = dot(na, pc);
 if( fabs(dv2) < 1e-5 ) // 0 actually
 {
  v = (dot(pos, nb) - dv0)/(dv1 - dot(pos, na));
 }
 else
 {
  qvx = (1.0/(2 * dv2)) * na;
  dvx = -dv1 / (2 * dv2);
  qvy = -(1.0/dv2) * nb;
  dvy = dv0/dv2;
  
  ka = dvx + dot(qvx, pos);
  kb = dvy + dot(qvy, pos); 
  
  tmp = sqrt(ka * ka - kb);
  v0 = ka - tmp;
  v1 = ka + tmp;
  
  if( v0 <= 1.0 && v0 >= 0)
   v = v0;
  else
   v = v1;
 } 
}


intercept_t PlanarConvexQuad::getIntercept(const ray_t &ray) const
{
 intercept_t intercept;
 
 // check whether ray intersects infinite plane
 intercept = InfinitePlane::getIntercept(ray);
 
 if(intercept.object == NULL) // no intersection
  return intercept;
 
 // ray intersects infinite plane. check whether intersection
 // inside quad
 vector3d_t n1, n2, n3, n4;
 n1 = cross((d_v2 - d_v1), (intercept.coord - d_v1));
 n2 = cross((intercept.coord - d_v1), (d_v4 - d_v1));
 n3 = cross((intercept.coord - d_v3), (d_v2 - d_v3));
 n4 = cross((d_v4 - d_v3), (intercept.coord - d_v3));
 
 if(dot(n1, n2) > 0 && dot(n3, n4) > 0 && dot(n2, n3) > 0) // intersection inside plane
 {
  intercept.normal = d_normal;
  intercept.object = (Object *)this;
 }
 else
  intercept.object = NULL;
 return intercept;
}


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


void Box::setName(char *name)
{
 d_name = "Box::";
 d_name += name; 
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


void ZCylinder::setName(char *name)
{
 d_name = "ZCylinder::";
 d_name += name; 
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


