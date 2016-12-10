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
   
  virtual void setName(char *name) {d_name = name;}
   // Assign a name to the object.
   //  name  A name to identify this object from another.

  virtual void setColor(rgb_t color) {d_color = color;}
   // Assign a color to the object
   //  color  The color of the object.

  virtual void translate(double x, double y, double z);
   // Set position of object w.r.t global coordinate system.
    
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
   
  virtual void setDiffuseLtRefCoeff(double kd) {d_kd = kd;}
   // Assign a diffuse light reflection coefficient
   //  kd  The diffuse light reflection coefficient usually in
   //      the range [0.0 - 1.0].
   
  virtual void setAmbLtRefCoeff(double ka) {d_ka = ka;}
   // Assign ambient light reflection coefficient
   //  ka  The ambient light reflection coefficient usually in
   //      the range [0.0 - 1.0].

  virtual void setSpecLtRefCoeff(double ks) {d_ks = ks;}
   // Assign a specular light reflection coefficient
   //  ks  The speclar light reflection coefficient usually in
   //      the range [0.0 - 1.0].

  virtual void setSpecLtRefExp(unsigned int n) {d_specRefExp = n;}
   // Assign a specular light reflection exponent. This factor
   // controls the sharpness of specular reflection
   //  n  The specular light reflection exponent usually in
   //      the range [0 - 400].

  virtual void setRefCoeff(double cref) {d_kRef = cref;}
   // Assign coefficent of light reflection
   
  virtual void setTransmissionCoeff(double ctrans) {d_kTrans = ctrans;}
   // Assign coefficient of light transmission through 
   // object

  virtual void setRefractiveIndex(double index) {d_refrInd = index;}
   // Assign refractive index of object
  
  virtual string getName() const {return d_name;}
   //  return  The name of the object.
   
  virtual rgb_t getColor(vector3d_t pos) const = 0;
   //  return  The color of the object.
 
  virtual double getDiffuseLtRefCoeff() const {return d_kd;};
   //  return  The diffuse light reflection coefficient.
   
  virtual double getAmbLtRefCoeff() const {return d_ka;}
    //  return  The ambient light reflection coefficient.

  virtual double getSpecLtRefCoeff() const {return d_ks;}
   //  return  The specular light reflection coefficient.

  virtual unsigned int getSpecLtRefExp() const {return d_specRefExp;}
   //  return  The specular light reflection exponent.

  virtual double getRefCoeff() const {return d_kRef;}
   //  return  Reflection coefficent of the object.
   
  virtual double getTransmissionCoeff() const {return d_kTrans;}
   //  return  Transmission coefficient of the object.

  virtual double getRefractiveIndex() const {return d_refrInd;}
   //  return  Refractive index of object

  virtual intercept_t getIntercept(const ray_t &ray) const = 0;
   //  return  The intercept of a ray on the object.
   //  ray     A ray from light source to the object.

 protected:
  string d_name;             // name of the object
  rgb_t d_color;             // color of the object
  double d_ka;               // ambient light reflection coeff
  double d_kd;               // diffuse light reflection coeff
  double d_ks;               // specular light reflection coeff
  unsigned int d_specRefExp; // specular light reflection exponent
  double d_kTrans;           // light transmission coefficient
  double d_refrInd;          // refractive index
  double d_kRef;             // light reflection coefficient
  double d_bumpiness;        // surface bumpiness factor
  Pixmap *d_texture;         // surface texture
  Pixmap *d_bumpMap;         // surface bump map
  bool d_hasTextureMap;      
  bool d_hasBumpMap;
  transform_t d_transform;   // trasformation matrix of object w.r.t
                             // global frame.
  transform_t d_iTransform;  // inverse transform 
};


//==================================================================
// class Sphere  A class for sperical geometric objects in 
//               the scene.
//==================================================================
class Sphere : public Object
{
 public:
  Sphere(vector3d_t center = vector3d_t(0,0,0), double radius = 0, 
         char *name = "Sphere");
  ~Sphere();
  virtual void setPosition(vector3d_t pos) {d_center = pos;}
  void setRadius(double r) {d_radius = r;}
  virtual vector3d_t getPosition() const {return d_center;}
  virtual void setName(char *name);
  virtual rgb_t getColor(vector3d_t pos) const;
  virtual intercept_t getIntercept(const ray_t &ray) const;
 private:
  void doInverseSphereMap(const vector3d_t pos, double &u, double &v) const;
  vector3d_t doSphereMap(double u, double v) const;
  vector3d_t getBumpedNormal(const intercept_t &intercept) const;

  vector3d_t d_center;
  double d_radius;
};


//==================================================================
// class InfinitePlane  A class for infinite planer objects in 
//                      the scene.
//==================================================================
class InfinitePlane : public Object
{
 public:
  InfinitePlane(vector3d_t normal = vector3d_t(0,-1,0), double distance = 0.5, 
        char *name ="InfinitePlane");
  ~InfinitePlane() {};
  void setPosition(vector3d_t normal, double d);
  virtual void setName(char *name);
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
  CheckerBoard(vector3d_t normal = vector3d_t(0,-1,0), double distance = 0.5, 
        char *name ="CheckerBoard") 
        : InfinitePlane(normal, distance, name) {d_checkerScale = 5.0;}
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
  virtual void setName(char *name);
  vector3d_t getNormal() {return d_normal;}
  virtual rgb_t getColor(vector3d_t pos) const;
  virtual intercept_t getIntercept(const ray_t &ray) const;
 private:
  vector3d_t d_normal;
};


//==================================================================
// class PlanarConvexQuad  A planar qudrilateral.
//==================================================================
class PlanarConvexQuad : public InfinitePlane
{
 public:
  PlanarConvexQuad(vector3d_t v1 = vector3d_t(0,0,0),  // four vertices
                   vector3d_t v2 = vector3d_t(0,0,0),  // specified 
                   vector3d_t v3 = vector3d_t(0,0,0),  // anticlockwise
                   vector3d_t v4 = vector3d_t(0,0,0),  // around normal
                   char *name = "PlanarConvexQuad");
  ~PlanarConvexQuad() {}
  virtual void setName(char *name);
  void setVertices(vector3d_t v1, vector3d_t v2, vector3d_t v3, vector3d_t v4);
  virtual rgb_t getColor(vector3d_t pos) const;
  virtual intercept_t getIntercept(const ray_t &ray) const;
 private:
  void doInverseConvQuadMap(const vector3d_t pos, double &u, double &v) const;

  vector3d_t d_v1, d_v2, d_v3, d_v4;
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
  virtual void setName(char *name);
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
  virtual void setName(char *name);
  virtual rgb_t getColor(vector3d_t pos) const;
  virtual intercept_t getIntercept(const ray_t &ray) const;
  void setEndCapsOn();
 private:
  vector3d_t d_center;
  double d_length;
  double d_radius;
  bool d_hasEndCaps;
};


#endif // ifndef _OBJECTS_HPP_INCLUDED
