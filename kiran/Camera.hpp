//==================================================================
// camera.hpp   The camera
//
// Author:      Vilas Kumar Chitrakaran (cvilas@ces.clemson.edu)
//              May 2004
//==================================================================

#ifndef _CAMERA_HPP_INCLUDED
#define _CAMERA_HPP_INCLUDED

#include "data_types.hpp"

//==================================================================
// class Camera  A standard camera
//==================================================================
class Camera
{
 public:
  Camera(vector3d_t pos = vector3d_t(0,0,0), 
         vector3d_t lookAt = vector3d_t(0,0,1), 
         vector3d_t up = vector3d_t(1,0,0));
   // The default constructor. Initializes parameters to 
   // following sensible defaults,
   //  position (0,0,0)
   //  Up Vector x axis 
   //  Direction z axis
   //  Far clipping plane = 1000 m
   //  focal distance = far clipping plane/2
   //  aperture = f/32;
   //  CCD size = 640 x 480
   
  ~Camera() {}
   // The destructor does nothing.
  
  void setPosition(vector3d_t pos, vector3d_t lookAt, vector3d_t up);
   // Camera location and orientation
   
  void setLensFocalLength(double f);
   // Lens focal length in mm (20, 28, 50, 90, 200, 300 mm, etc)
   
  void setAperture(double a);
  	// Set aperture in terms of f-stops, example 1, 1.4, 2, 2.8, 4, 
    // 5.6, 8, 11, 16, 22, 32, 45 ...  

  void setCcdSize(int w, int h);
   // Image dimensions
   
  void setFocalDistance(double d);
   // Distance where camera is focussed at.
  
  void setFarClippingDistance(double far);
   // Set the farthest distance visible to the camera.
   
  double getFarClippingDistance() const;
   // Farthest distance visible to the camera.
   
  ray_list_t *getRays(double u, double v) const; 
   //  return  A bunch of rays from camera for the pixel 
   //          location (u,v). 

 private:
  double d_focalLength;
  double d_lensRadius;
  double d_aperture;
  double d_focus;
  int d_width;
  int d_height;
  double d_uo;
  double d_vo;
  double d_ku;
  double d_kv;
  double d_farthest;
  vector3d_t d_look;
  vector3d_t d_pos;
  vector3d_t d_up;
  vector3d_t d_right;
};


#endif //_CAMERA_HPP_INCLUDED

