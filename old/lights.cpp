//==================================================================
// lights.cpp   Data types for lights
//
// Author:      Vilas Kumar Chitrakaran (cvilas@ces.clemson.edu)
//              May 2004
//==================================================================

#include "lights.hpp"


//==================================================================
// class Light
//==================================================================

Light::Light()
{
 d_name="unknown light";
}


void Light::setName(char *name)
{
 d_name = name; 
}


void Light::setIntensity(double r, double g, double b)
{
 d_intensity = vector3d_t(r,g,b);
}


void Light::setIntensityAttnFactors(double f1, double f2, double f3)
{
 d_af1 = f1;
 d_af2 = f2;
 d_af3 = f3;
} 


void Light::setPosition(const vector3d_t &pos)
{
 d_pos = pos;
}


void Light::setDirection(const vector3d_t &dir)
{
 d_dir = dir;
}


vector3d_t Light::getPosition() const
{
 return d_pos;
}

  
vector3d_t Light::getDirection() const
{
 return d_dir;
}


string Light::getName() const 
{
 return d_name;
}


double Light::getAttnFactor(double distance) const
{
 double fatt;

 fatt = 1.0/(d_af1 + d_af2 * distance + d_af3 * pow(distance, 2));
 
 if (fatt > 1)
  fatt = 1;

 if (fatt < 0.0)
  fatt = 0;

 return fatt;
}


//==================================================================
// class PointLight
//==================================================================
PointLight::PointLight(vector3d_t pos, double r, double g, double b, char *name)
{
 d_pos = pos;
 d_intensity = vector3d_t(r,g,b);
 d_name = name; 
 d_af1 = 0.1;
 d_af2 = 0.1;
 d_af3 = 1;
}


rgb_t PointLight::calculateLight(intercept_t intercept) const
{
 rgb_t totalColor;
 double diffR, diffG, diffB, specLight = 0.0;
 double kd;
 double ks;
 double kr, kt;
 unsigned int specRefExp;
 double fatt;
 double nDotL, eDotR;
 rgb_t objectColor;
 vector3d_t lightDir;
 vector3d_t specRefn;
 vector3d_t normal;
 
 kd = intercept.object->getDiffuseLtRefCoeff();
 ks = intercept.object->getSpecLtRefCoeff();
 kr = intercept.object->getRefCoeff();
 kt = intercept.object->getTransmissionCoeff();
 specRefExp = intercept.object->getSpecLtRefExp();
 objectColor = intercept.object->getColor(intercept.coord);
 fatt = getAttnFactor( norm(intercept.coord - d_pos) );

 lightDir = getPosition();
 lightDir = lightDir - intercept.coord;
 lightDir = normalize(lightDir);
 
 normal = normalize(intercept.normal); // just in case.
 nDotL = dot(lightDir, normal);

 if(nDotL < 0)
  nDotL = 0; 

 //---------- diffuse light ----------
 diffR = kd * nDotL * (double)objectColor.r;
 diffG = kd * nDotL * (double)objectColor.g;
 diffB = kd * nDotL * (double)objectColor.b;
 
 //---------- specular light ----------
 specRefn.x = 2 * nDotL * normal.x - lightDir.x;
 specRefn.y = 2 * nDotL * normal.y - lightDir.y;
 specRefn.z = 2 * nDotL * normal.z - lightDir.z;
 specRefn = normalize(specRefn);

 eDotR = - dot(specRefn, intercept.incidentRay);

 if(eDotR < 0 )
  eDotR = 0; 

 if(nDotL <= 0 )
  specLight = 0;
 else
  specLight = ks * pow(eDotR, (int)specRefExp);

 //---------- add up ----------
 totalColor.r = fatt * d_intensity.x * ((1 - kr - kt) * diffR + specLight);
 if(totalColor.r > 1.0) totalColor.r = 1.0;
 totalColor.g = fatt * d_intensity.y * ((1 - kr - kt) * diffG + specLight);
 if(totalColor.g > 1.0) totalColor.g = 1.0;
 totalColor.b = fatt * d_intensity.z * ((1 - kr - kt) * diffB + specLight);
 if(totalColor.b > 1.0) totalColor.b = 1.0;
 return totalColor;
}


//==================================================================
// class AmbientLight
//==================================================================

AmbientLight::AmbientLight(double r, double g, double b, char *name)
{
 d_intensity = vector3d_t(r,g,b);
 d_name += name; 
 d_af1 = 1;
 d_af2 = 0;
 d_af3 = 0;
}


rgb_t AmbientLight::calculateLight(intercept_t intercept) const
{
 rgb_t result;
 double ka, kr, kt;
 rgb_t objectColor;
 
 ka = intercept.object->getAmbLtRefCoeff();
 kr = intercept.object->getRefCoeff();
 kt = intercept.object->getTransmissionCoeff();
 objectColor = intercept.object->getColor(intercept.coord);
 
 result.r = d_intensity.x * ka * (1 - kr - kt) * (double)objectColor.r;
 if(result.r > 1.0) result.r = 1.0;
 result.g = d_intensity.y * ka * (1 - kr - kt) * (double)objectColor.g;
 if(result.g > 1.0) result.g = 1.0;
 result.b = d_intensity.z * ka * (1 - kr - kt) * (double)objectColor.b;
 if(result.b > 1.0) result.b = 1.0;

 return result;
}

