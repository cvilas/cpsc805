//==================================================================
// objects.hpp  Data types for geometric objects
//
// Author:      Vilas Kumar Chitrakaran (cvilas@ces.clemson.edu)
//              Feb 2004
//==================================================================

#ifndef _OBJECTS_HPP_INCLUDED
#define _OBJECTS_HPP_INCLUDED

#include "data_types.hpp"
#include "Pixmap.hpp"

//==================================================================
// class Object  A pure virtual base class for geometric objects in 
//               the scene.
//==================================================================
class Object
{
 public:
  Object();
   // The default constructor. 
   
  virtual ~Object();
   // The default destructor. Does nothing.
   
  virtual void setName(const char *name) {d_name = name;}
   // Assign a name to the object.
   //  name  A name to identify this object from another.

  virtual void setColor(rgb_t color) {d_color = color;}
   // Assign a color to the object
   //  color  The color of the object.

  virtual void translate(double x, double y, double z);
   // Move position of object w.r.t local coordinate system.
    
  virtual void rotate(double x, double y, double z);
   // Rotate the object about its own local frame.
  
  virtual void setTexture(char *ppmFileName);
   // Set a texture to object surface
   //  ppmFileName  Texture image
   
  virtual void setBumpMap(char *ppmFileName);
   // Set a bump map to object surface
   //  ppmFileName  Bump map image

  virtual void setBumpiness(double bumpiness) {d_bumpiness = bumpiness;}
   // Set the bumpiness of the surface. 
   
  virtual void setDiffuseLtCoeff(double kd) {d_kd = kd;}
   // Assign a diffuse light reflection coefficient
   //  kd  The diffuse light reflection coefficient usually in
   //      the range [0.0 - 1.0].
   
  virtual void setAmbLtCoeff(double ka) {d_ka = ka;}
   // Assign ambient light reflection coefficient
   //  ka  The ambient light reflection coefficient usually in
   //      the range [0.0 - 1.0].

  virtual void setSpecLtCoeff(double ks) {d_ks = ks;}
   // Assign a specular light reflection coefficient
   //  ks  The speclar light reflection coefficient usually in
   //      the range [0.0 - 1.0].

  virtual void setSpecLtExp(unsigned int n) {d_specRefExp = n;}
   // Assign a specular light reflection exponent. This factor
   // controls the sharpness of specular reflection
   //  n  The specular light reflection exponent usually in
   //      the range [0 - 400].

  virtual void setReflectivity(double cref) {d_kRef = cref;}
   // Assign coefficent of light reflection
   
  virtual void setTransmittivity(double ctrans) {d_kTrans = ctrans;}
   // Assign coefficient of light transmission through 
   // object

  virtual void setRefractiveIndex(double index) {d_refrInd = index;}
   // Assign refractive index of object
  
  virtual string getName() const {return d_name;}
   //  return  The name of the object.
   
  virtual rgb_t getColor(vector3d_t pos) const = 0;
   //  return  The color of the object.
 
  virtual double getDiffuseLtCoeff() const {return d_kd;};
   //  return  The diffuse light reflection coefficient.
   
  virtual double getAmbLtCoeff() const {return d_ka;}
    //  return  The ambient light reflection coefficient.

  virtual double getSpecLtCoeff() const {return d_ks;}
   //  return  The specular light reflection coefficient.

  virtual unsigned int getSpecLtExp() const {return d_specRefExp;}
   //  return  The specular light reflection exponent.

  virtual double getReflectivity() const {return d_kRef;}
   //  return  Reflection coefficent of the object.
   
  virtual double getTransmittivity() const {return d_kTrans;}
   //  return  Transmission coefficient of the object.

  virtual double getRefractiveIndex() const {return d_refrInd;}
   //  return  Refractive index of object

  virtual intercept_t getIntercept(const ray_t &ray) const = 0;
   //  return  The intercept of a ray on the object.
   //  ray     A ray from light source to the object.

 protected:
  string d_name;             // Name of the object
  rgb_t d_color;             // Color of the object
  double d_ka;               // Ambient light reflection coeff
  double d_kd;               // Diffuse light reflection coeff
  double d_ks;               // Specular light reflection coeff
  unsigned int d_specRefExp; // Specular light reflection exponent
  double d_kTrans;           // Light transmission coefficient
  double d_refrInd;          // Refractive index
  double d_kRef;             // Light reflection coefficient
  double d_bumpiness;        // Surface bumpiness factor
  Pixmap *d_texture;         // Surface texture
  Pixmap *d_bumpMap;         // Surface bump map
  bool d_hasTextureMap;      
  bool d_hasBumpMap;
  transform_t d_transform;   // Trasformation matrix of object w.r.t
                             // global frame.
  transform_t d_iTransform;  // Inverse transform 
};


//==================================================================
// class Sphere  A class for sperical geometric objects in 
//               the scene.
//==================================================================
class Sphere : public Object
{
 public:
  Sphere(double radius = 0);
  ~Sphere() {};
  void setRadius(double r) {d_radius = r;}
  virtual rgb_t getColor(vector3d_t pos) const;
  virtual intercept_t getIntercept(const ray_t &ray) const;
 private:
  void doInverseSphereMap(const vector3d_t pos, double &u, double &v) const;
  vector3d_t doSphereMap(double u, double v) const;
  vector3d_t getBumpedNormal(const intercept_t &intercept) const;

  double d_radius;
};


//==================================================================
// class InfinitePlane  A class for infinite planer objects in 
//                      the scene.
//==================================================================
class InfinitePlane : public Object
{
 public:
  InfinitePlane();
  ~InfinitePlane() {};
  virtual void translate(double x, double y, double z);
  virtual void rotate(double x, double y, double z);
  vector3d_t getNormal() const {return d_normal;}
  double getDistanceFromOrigin() const {return d_distance;}
  virtual rgb_t getColor(vector3d_t pos) const;
  virtual intercept_t getIntercept(const ray_t &ray) const;
 protected:
  vector3d_t d_normal;
  double d_distance;
};


//==================================================================
// class CheckerBoard  A special case of class InifintePlane with 
//                     checker board pattern.
//==================================================================
class CheckerBoard : public InfinitePlane
{
 public:
  CheckerBoard() : InfinitePlane() {d_checkerScale = 5.0;}
  ~CheckerBoard() {}
  virtual void setColor(rgb_t color) {d_color = color;}
  virtual void setColor2(rgb_t color) {d_color2 = color;}
  void setCheckerSize(double side) {d_checkerScale = side;}
  virtual rgb_t getColor(vector3d_t pos) const;
 private:
  double d_checkerScale;
  rgb_t d_color2;
};


//==================================================================
// class PlanarPolygon  A finitely large plane.
//==================================================================
class PlanarPolygon : public InfinitePlane
{
 public:
  PlanarPolygon(vector3d_t *v1=NULL, int numPoints = 0, char *name="Polygon");
  ~PlanarPolygon();
  vector3d_t getNormal() {return d_normal;}
  virtual rgb_t getColor(vector3d_t pos) const;
  virtual intercept_t getIntercept(const ray_t &ray) const;
 private:
  vector3d_t d_normal;
};


//==================================================================
// class PlanarConvexQuad  A planar qudrilateral.
//                         The local axis is located at the center of
//                         gravity of the plane. The local x axis is 
//                         along normal to the plane, the local 
//                         y axis is along the direction from center
//                         of gravity to first vertex v1 and z axis is
//                         along a mutually perpendicular direction
//                         to both local X and local Y.
//==================================================================
class PlanarConvexQuad : public InfinitePlane
{
 public:
  PlanarConvexQuad() {};
  ~PlanarConvexQuad() {}
  virtual void translate(double x, double y, double z);
  virtual void rotate(double x, double y, double z);
  void setVertices(vector3d_t v1, vector3d_t v2, vector3d_t v3, vector3d_t v4);
  virtual rgb_t getColor(vector3d_t pos) const;
  virtual intercept_t getIntercept(const ray_t &ray) const;
 private:
  vector3d_t findCog() const;
  void doInverseConvQuadMap(const vector3d_t pos, double &u, double &v) const;
  vector3d_t getBumpedNormal(const intercept_t &intercept) const;

  vector3d_t d_v1, d_v2, d_v3, d_v4;
  vector3d_t d_vl1, d_vl2, d_vl3, d_vl4; // vertices in local coords
};


//==================================================================
// class Box  The box parallel to cardinal axes
//==================================================================
class Box : public Object
{
 public:
  Box(vector3d_t lo = vector3d_t(0,0,10), vector3d_t hi = vector3d_t(0,0,10), 
      char *name="Box");
  ~Box();
  void setVertices(vector3d_t lo, vector3d_t hi);
  virtual rgb_t getColor(vector3d_t pos) const;
  virtual intercept_t getIntercept(const ray_t &ray) const;
 private:
  vector3d_t d_vl;
  vector3d_t d_vh;
};


//==================================================================
// class ZCylinder  Cylinders aligned along Z
//==================================================================
class ZCylinder : public Object
{
 public:
  ZCylinder(vector3d_t center = vector3d_t(0,0,0), double r = 0,
            double extent = 0, char *name = "ZCylinder");
  ~ZCylinder() {}
  virtual void setPosition(vector3d_t pos) {d_center = pos;}
  virtual void setRadius(double r) {d_radius = r;}
  virtual void setLength(double l) {d_length = fabs(l);}
  virtual rgb_t getColor(vector3d_t pos) const;
  virtual intercept_t getIntercept(const ray_t &ray) const;
  void setEndCapsOn();
 private:
  vector3d_t d_center;
  double d_length;
  double d_radius;
  bool d_hasEndCaps;
};


//==================================================================
// findLineLineIntercept   Find the intercept of two lines, one from 
//                         v1 passing through v2 and the other from
//                         v3 passing through v4. The lines are
//                         assumed to lie on a plane and not parall-
//                         -el. There are no checks and hence inval-
//                         -id coordinates will result in gibberish
//                         return values.
//==================================================================
vector3d_t findLineLineIntercept(const vector3d_t &v1, const vector3d_t &v2,
                                 const vector3d_t &v3, const vector3d_t &v4);


//==================================================================
// findTriangleCog   Find center of gravity of a triangular patch.
//==================================================================
vector3d_t findTriangleCog(const vector3d_t &v1, const vector3d_t &v2, 
                           const vector3d_t &v3);

#endif // ifndef _OBJECTS_HPP_INCLUDED
