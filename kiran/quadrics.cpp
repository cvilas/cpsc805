#include "objects.hpp"

//==================================================================
// class Sphere
//==================================================================

Sphere::Sphere(double radius)
{
 setRadius(d_radius);
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
 vector3d_t center;
 ray_t lray;
 transform_t tr;
 
 tr = d_iTransform;
 lray.orig = tr * ray.orig;
 tr.t[0][3] = tr.t[1][3] = tr.t[2][3] = 0;
 lray.dir = tr * ray.dir;
 
 center = get_translation(d_transform);

 a = pow( norm(lray.dir), 2);
 b = 2 * (dot(lray.dir, lray.orig));
 c = pow( norm(lray.orig), 2) - pow(d_radius, 2);

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
  intercept.normal = intercept.coord - center;
  intercept.normal = normalize(intercept.normal);
  if(d_hasBumpMap)
   intercept.normal = getBumpedNormal(intercept);
  return intercept;
 }
 
 (t1 > 0.0001 ) ? (t = t1) : (t = t2); // eye inside sphere
 intercept.coord = ray.orig + ray.dir * t;
 intercept.normal = intercept.coord - center;
 intercept.normal = normalize(intercept.normal);
 if(d_hasBumpMap)
  intercept.normal = getBumpedNormal(intercept);
 return intercept;
}


void Sphere::doInverseSphereMap(const vector3d_t pos, double &u, double &v) const
{
 vector3d_t sn, sp, se;
 double phi, theta = 0, ut;
 vector3d_t lpos;
 
 lpos = d_iTransform * pos;
 sn = normalize(lpos); // surface normal at pos
 sp = vector3d_t(1,0,0); // north pole
 se = vector3d_t(0,1,0); // equator
 
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
 vector = d_transform * vector;
 return vector;
}


vector3d_t Sphere::getBumpedNormal(const intercept_t &intercept) const
{
 if(!d_hasBumpMap)
  return intercept.normal;
  
 vector3d_t newNormal, pu, pd, pl, pr;
 vector3d_t center;
 double u, v, ut, vt;
 rgb_t bump;
 double bumpiness;
 
 center = get_translation(d_transform);
 doInverseSphereMap(intercept.coord, u, v); // get u, v for a intersection

 // create 4 new normals depending respectively on 
 // pixels values up, down, left and right of this pixel
 // in the bump map.
 ut = u-2.0/d_bumpMap->width();
 if(ut < 0.0) ut = 0.0;
 bump = (*d_bumpMap)((int)(ut * (d_bumpMap->width()-1) + 1), 
                      (int)((1-v) * (d_bumpMap->height()-1))+1);
 bumpiness = 0.33 * (bump.r + bump.g + bump.b) - 0.5;
 pl = doSphereMap(ut, v);
 pl = bumpiness * normalize(pl - center);

 ut = u+2.0/d_bumpMap->width();
 if(ut > 1.0) ut = 1.0;
 bump = (*d_bumpMap)((int)(ut * (d_bumpMap->width()-1) + 1), 
                      (int)((1-v) * (d_bumpMap->height()-1))+1);
 bumpiness = 0.33 * (bump.r + bump.g + bump.b) - 0.5;
 pr = doSphereMap(ut, v);
 pr = bumpiness * normalize(pr - center);
 
 vt = v+2.0/d_bumpMap->height();
 if(vt > 1.0) vt = 1.0;
 bump = (*d_bumpMap)((int)(u * (d_bumpMap->width()-1) + 1), 
                      (int)((1-vt) * (d_bumpMap->height()-1))+1);
 bumpiness = 0.33 * (bump.r + bump.g + bump.b) - 0.5;
 pu = doSphereMap(u, vt);
 pu = bumpiness * normalize(pu - center);
 
 vt = v-2.0/d_bumpMap->height();
 if(vt < 0.0) vt = 0.0;
 bump = (*d_bumpMap)((int)(u * (d_bumpMap->width()-1) + 1), 
                      (int)((1-vt) * (d_bumpMap->height()-1))+1);
 bumpiness = 0.33 * (bump.r + bump.g + bump.b) - 0.5;
 pd = doSphereMap(u, vt);
 pd = bumpiness * normalize(pd - center);

 newNormal = d_bumpiness * (pl + pr + pu + pd);
 newNormal = normalize(intercept.normal + newNormal);
 
 return newNormal;
}


