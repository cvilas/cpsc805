//==================================================================
// planes.cpp   Planes
//
// Author:      Vilas Kumar Chitrakaran (cvilas@ces.clemson.edu)
//              May 2004
//==================================================================


#include "objects.hpp"

//==================================================================
// class InfinitePlane
//==================================================================

InfinitePlane::InfinitePlane()
{
 d_normal = vector3d_t(1,0,0);
 d_distance = 0;
}


void InfinitePlane::translate(double x, double y, double z)
{
 vector3d_t pos;
 Object::translate(x,y,z);
 pos = get_translation(d_transform);
 d_normal.x = d_transform.t[0][0];
 d_normal.y = d_transform.t[0][1];
 d_normal.z = d_transform.t[0][2];
 d_distance = -dot(d_normal, pos);
}


void InfinitePlane::rotate(double x, double y, double z)
{
 vector3d_t pos;
 Object::rotate(x,y,z);
 pos = get_translation(d_transform);
 d_normal.x = d_transform.t[0][0];
 d_normal.y = d_transform.t[0][1];
 d_normal.z = d_transform.t[0][2];
 d_distance = -dot(d_normal, pos);
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
 
 ray_t lray;
 transform_t tr;
 tr = d_iTransform;
 lray.orig = tr * ray.orig;
 tr.t[0][3] = tr.t[1][3] = tr.t[2][3] = 0;
 lray.dir = tr * ray.dir;
 lray.dir = normalize(lray.dir);

 intercept.object = NULL;
 nDotR = lray.dir.x;
 nDotE = lray.orig.x;
 
 if( fabsf(nDotR) <= 1e-5 ) // ray parallel to plane
  return intercept;
 
 t = (-1.0) * (nDotE / nDotR);
 
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
void PlanarConvexQuad::translate(double x, double y, double z)
{
 vector3d_t pos;
 transform_t t;
 InfinitePlane::translate(x,y,z);
 pos = get_translation(d_transform);
 t = d_transform;
 t.t[0][3] = t.t[1][3] = t.t[2][3] = 0.0;
 d_v1 = pos + t * d_vl1;
 d_v2 = pos + t * d_vl2;
 d_v3 = pos + t * d_vl3;
 d_v4 = pos + t * d_vl4;
}


void PlanarConvexQuad::rotate(double x, double y, double z)
{
 vector3d_t pos;
 transform_t t;
 InfinitePlane::rotate(x,y,z);
 pos = get_translation(d_transform);
 t = d_transform;
 t.t[0][3] = t.t[1][3] = t.t[2][3] = 0.0;
 d_v1 = pos + t * d_vl1;
 d_v2 = pos + t * d_vl2;
 d_v3 = pos + t * d_vl3;
 d_v4 = pos + t * d_vl4;
}


vector3d_t PlanarConvexQuad::findCog() const
{
 vector3d_t c1, c2, c3, c4, c;
 c1 = findTriangleCog(d_v1, d_v2, d_v4);
 c2 = findTriangleCog(d_v2, d_v3, d_v4);
 c3 = findTriangleCog(d_v1, d_v2, d_v3);
 c4 = findTriangleCog(d_v1, d_v3, d_v4);
 c = findLineLineIntercept(c1, c2, c3, c4);
 return c;
}


void PlanarConvexQuad::setVertices(vector3d_t v1, vector3d_t v2, 
                                   vector3d_t v3, vector3d_t v4)
{
 vector3d_t pos, localZ, localY;
 transform_t t;
 d_v1 = v1;
 d_v2 = v2;
 d_v3 = v3;
 d_v4 = v4;
 d_normal = cross((v2 - v1), (v4 - v1));
 d_normal = normalize(d_normal);
 d_distance = -1.0 * dot(d_normal, v1);
 pos = findCog();
 d_transform.t[0][0] = d_normal.x;
 d_transform.t[1][0] = d_normal.y;
 d_transform.t[2][0] = d_normal.z;
 localY = v1 - pos;
 localY = normalize(localY);
 d_transform.t[0][1] = localY.x;
 d_transform.t[1][1] = localY.y;
 d_transform.t[2][1] = localY.z;
 localZ = cross(d_normal, localY);
 localZ = normalize(localZ);
 d_transform.t[0][2] = localZ.x;
 d_transform.t[1][2] = localZ.y;
 d_transform.t[2][2] = localZ.z;
 d_transform.t[0][3] = pos.x;
 d_transform.t[1][3] = pos.y;
 d_transform.t[2][3] = pos.z;
 d_iTransform = inverse(d_transform);

 t = d_iTransform;
 t.t[0][3] = t.t[1][3] = t.t[2][3] = 0.0;
 d_vl1 = t * (v1 - pos);
 d_vl2 = t * (v2 - pos);
 d_vl3 = t * (v3 - pos);
 d_vl4 = t * (v4 - pos);
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
 
 if(dot(n1, n2) > 0 && dot(n3, n4) > 0 && dot(n2, n3) > 0) 
 { // intersection inside plane
  intercept.normal = getBumpedNormal(intercept);
  intercept.object = (Object *)this;
 }
 else
  intercept.object = NULL;
 return intercept;
}


vector3d_t PlanarConvexQuad::getBumpedNormal(const intercept_t &intercept) const
{
 if(!d_hasBumpMap)
  return intercept.normal;
 
 vector3d_t bumpedNormal, tmp, n1, n2, n3, n4;
 rgb_t bump;
 double bumpiness, u, v;
 
 // look at 0.1mm along +y
 tmp.x = d_transform.t[0][1];
 tmp.y = d_transform.t[1][1];
 tmp.z = d_transform.t[2][1];
 tmp = intercept.coord + (tmp * 0.001);
 doInverseConvQuadMap(tmp, u, v);
 bump = (*d_bumpMap)((int)(u * (d_bumpMap->width()-1) + 1), 
                      (int)((1-v) * (d_bumpMap->height()-1))+1);
 bumpiness = 0.33 * (bump.r + bump.g + bump.b) - 0.5;
 n1 = bumpiness * (tmp - intercept.coord);

 // look at 0.1mm along -y
 tmp.x = d_transform.t[0][1];
 tmp.y = d_transform.t[1][1];
 tmp.z = d_transform.t[2][1];
 tmp = intercept.coord - (tmp * 0.001);
 doInverseConvQuadMap(tmp, u, v);
 bump = (*d_bumpMap)((int)(u * (d_bumpMap->width()-1) + 1), 
                      (int)((1-v) * (d_bumpMap->height()-1))+1);
 bumpiness = 0.33 * (bump.r + bump.g + bump.b) - 0.5;
 n2 = bumpiness * (tmp - intercept.coord);

 // look at 0.1mm along +z
 tmp.x = d_transform.t[0][2];
 tmp.y = d_transform.t[1][2];
 tmp.z = d_transform.t[2][2];
 tmp = intercept.coord + (tmp * 0.001);
 doInverseConvQuadMap(tmp, u, v);
 bump = (*d_bumpMap)((int)(u * (d_bumpMap->width()-1) + 1), 
                      (int)((1-v) * (d_bumpMap->height()-1))+1);
 bumpiness = 0.33 * (bump.r + bump.g + bump.b) - 0.5;
 n3 = bumpiness * (tmp - intercept.coord);

 // look at 0.1mm along -z
 tmp.x = d_transform.t[0][2];
 tmp.y = d_transform.t[1][2];
 tmp.z = d_transform.t[2][2];
 tmp = intercept.coord - (tmp * 0.001);
 doInverseConvQuadMap(tmp, u, v);
 bump = (*d_bumpMap)((int)(u * (d_bumpMap->width()-1) + 1), 
                      (int)((1-v) * (d_bumpMap->height()-1))+1);
 bumpiness = 0.33 * (bump.r + bump.g + bump.b) - 0.5;
 n4 = bumpiness * (tmp - intercept.coord);

 bumpedNormal = (d_bumpiness * 100000.0 * (n1+n2+n3+n4)) + d_normal;
 bumpedNormal = normalize(bumpedNormal);
 //cout << "bumped: " << bumpedNormal.x << " " << bumpedNormal.y << " " << bumpedNormal.z << endl;
 //cout << "orig: " << d_transform.t[0][1] << " " << d_transform.t[1][1] << " " << d_transform.t[2][1] << endl;
 return bumpedNormal;
}


