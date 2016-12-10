//==================================================================
// camera.cpp   The camera
//
// Author:      Vilas Kumar Chitrakaran (cvilas@ces.clemson.edu)
//              May 2004
//==================================================================

#include "camera.hpp"


Camera::Camera(vector3d_t pos, vector3d_t lookAt, vector3d_t up)
{
 // Set sensible defaults
 setPosition(pos, lookAt, up);
 setCcdSize(640, 480);

 d_focalLength = 50E-3;
 d_farthest = 1000;
 d_focus = d_farthest/2.0;
 d_aperture = 32.0;
 d_lensRadius = d_focalLength/(2 * d_aperture);
}


void Camera::setLensFocalLength(double f) 
{
 d_focalLength = f;
 d_lensRadius = d_focalLength/(2 * d_aperture);
} 


void Camera::setAperture(double a) 
{
 d_aperture = a;
 d_lensRadius = d_focalLength/(2 * d_aperture);
}


void Camera::setFocalDistance(double d)
{
 d_focus = d;
}


void Camera::setCcdSize(int w, int h)
{
 d_width = w;
 d_height = h;
 
 d_uo = d_width/2;
 d_vo = d_height/2;
 d_uo = d_width/2;
 d_vo = d_height/2;
 d_ku = 70E-3/(d_width + d_height); // 35mm film dimensions
 d_kv = 70E-3/(d_width + d_height);
}


void Camera::setFarClippingDistance(double far)
{
 d_farthest = far;
}


double Camera::getFarClippingDistance() const 
{
 return d_farthest;
}


void Camera::setPosition(vector3d_t pos, vector3d_t lookAt, vector3d_t up)
{
 d_pos = pos;
 d_look = lookAt - pos;
 d_right = cross(d_look, up);
 d_up = cross(d_right, d_look);
 
 d_look = normalize(d_look);
 d_up = normalize(d_up);
 d_right = normalize(d_right);
}


ray_list_t *Camera::getRays(double u, double v) const
{
 ray_list_t *rayList = NULL, *tmpPtr = NULL;
 double l;
 vector3d_t vx, vy, vz, centerRay;
 vector3d_t fPoint;
 vector3d_t cvec[8];
 
 if(u > d_width+0.5 || u < -0.5 || v > d_height+0.5 || v < -0.5)
 {
  cerr << "Camera: ERROR pixel index (" << u << ", " << v << ") out of range." 
       << endl;
  return NULL;
 }
 
 vz = d_focalLength * d_look;
 vy = (d_ku * (d_uo - u + 0.5)) * d_right;
 vx = (d_kv * (v - d_vo + 0.5)) * d_up;
 centerRay = vx + vy - vz;
 
 centerRay = normalize(centerRay);

 fPoint = d_pos - (centerRay * d_focus);

 // vectors in 8 directions along lens surface
 cvec[0] = d_up;
 cvec[1] = normalize(d_up + d_right);
 cvec[2] = d_right;
 cvec[3] = normalize(d_right - d_up);
 cvec[4] = -1.0 * cvec[0];
 cvec[5] = -1.0 * cvec[1];
 cvec[6] = -1.0 * cvec[2];
 cvec[7] = -1.0 * cvec[3]; 

 l = d_focalLength/90.0; // only center ray at f-stop of 45+
 rayList = new ray_list_t;
 tmpPtr = rayList;

 // generate rays from lens surface
 for(double j = l; j < d_lensRadius; j += l)
 {
  for(int i = 0; i < 8; i++)
  {
   tmpPtr->ray.orig = d_pos + (cvec[i] * j);
   tmpPtr->ray.dir = fPoint - tmpPtr->ray.orig;
   tmpPtr->ray.dir = normalize(tmpPtr->ray.dir);
   tmpPtr->nxt = new ray_list_t;
   tmpPtr = tmpPtr->nxt;
  }
 }

 tmpPtr->ray.orig = d_pos;
 tmpPtr->ray.dir = fPoint - tmpPtr->ray.orig;
 tmpPtr->ray.dir = normalize(tmpPtr->ray.dir);
 
 return rayList;
}


