//==================================================================
// lights.hpp   Data types for lights
//
// Author:      Vilas Kumar Chitrakaran (cvilas@ces.clemson.edu)
//              May 2004
//==================================================================

#ifndef _LIGHTS_HPP_INCLUDED
#define _LIGHTS_HPP_INCLUDED

#include "data_types.hpp"
#include "objects.hpp"
#include <math.h>

//==================================================================
// class Light  A pure virtual base class for all lights.
//==================================================================
class Light
{
 public:
  Light();
   // The default constructor.
   
  virtual ~Light() {}
   // The default destructor. Does nothing.
   
  virtual void setName(char *name);
   // Assign a name to the light source.
   //  name  A name to identify this light source from another.
   
  virtual void setIntensity(double r, double g, double b);
   // Set the intensity of the light source
   //  r,g,b  RGB triplet, each usually in the range [0.0 - 1.0].
  
  virtual void setIntensityAttnFactors(double f1, double f2, double f3); 
   // Set the fudge factors for light attenuation
   //  f1  Constant attenuation factor
   //  f2  First order attenuation factor
   //  f3  Second order attenuation factor
   
  virtual void setPosition(const vector3d_t &pos);
   // Set position of object w.r.t global coordinate system.
    
  virtual void setDirection(const vector3d_t &dir);
   // Set the direction of light. Has no meaning for
   // omni-directional light.
   
  virtual vector3d_t getPosition() const;
   //  return  Position in global coordinate system.
  
  virtual vector3d_t getDirection() const;
   //  return  Direction of light. Has no meaning for
   //          omnidirectional lights
   
  virtual string getName() const;
   //  return  The name of the light source.
   
  virtual double getAttnFactor(double distance) const;
   // return  A value in the range [0-1] for fraction
   //         of light reaching a point at the specified 
   //         distance.
    
  virtual rgb_t calculateLight(intercept_t intercept) const = 0;
   // Calculate intensity of light from this source at the
   // intercept point on an object.
   //  return  RGB light intensity in the range [0-255].

 protected:
  string d_name; // name for the source
  vector3d_t d_intensity;
  double d_af1;  // constant fudge factors for intensity attenuation
  double d_af2; 
  double d_af3;
  vector3d_t d_pos; // position
  vector3d_t d_dir; // direction
};


//==================================================================
// class PointLight  A class for point light sources.
//==================================================================
class PointLight : public Light
{
 public:
  PointLight(vector3d_t pos = vector3d_t(0,0,0), 
        double r = 1, double g = 1, double b = 1, 
        char *name = "Point Light");
  ~PointLight() {}
  virtual rgb_t calculateLight(intercept_t intercept) const;

 private:
};


//==================================================================
// class AmbientLight  A class for ambient light sources.
//==================================================================
class AmbientLight : public Light
{
 public:
  AmbientLight(double r = 1, double g = 1, double b = 1, 
               char *name = "Ambient Light");
  ~AmbientLight() {}
  virtual rgb_t calculateLight(intercept_t intercept) const;

 private:
};


#endif //_LIGHTS_HPP_INCLUDED
