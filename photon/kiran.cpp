//==================================================================
// kiran.cpp       Ray-tracer version 1.3.1
//                 (With Photon Mapping hack)
//
// Author:         Vilas Kumar Chitrakaran (cvilas@ces.clemson.edu)
//                 May 1, 2004
//==================================================================

#include "data_types.hpp"
#include "objects.hpp"
#include "Pixmap.hpp"
#include "SceneReader.hpp"

#include <signal.h>
#include <vector>
#include <unistd.h>
#include <math.h>

using namespace std;

//==================================================================
// kiran_find_intercept
//==================================================================
intercept_t kiran_find_intercept(ray_t ray, const vector<Object *> &sceneList, 
                                 double tooClose, double tooFar)
{
 intercept_t visibleIntercept, intercept;
 double distance;
 visibleIntercept.coord = vector3d_t(tooFar,tooFar,tooFar);
 visibleIntercept.object = NULL;

 for(unsigned int o = 0; o < sceneList.size(); o++)
 {
  intercept = sceneList[o]->getIntercept(ray);

  // did we hit something
  if( intercept.object == NULL )
   continue;
  
  // check if intercept is too near or too far
  distance = norm(intercept.coord - ray.orig);
  if( (distance < tooClose) || (distance > tooFar) )
   continue;
    
  // check if smallest intercept yet
  if( distance < norm(visibleIntercept.coord - ray.orig) )
   visibleIntercept = intercept;
 }
 return visibleIntercept;
}


//==================================================================
// kiran_do_lights
//==================================================================
rgb_t kiran_do_lights(intercept_t intercept, const vector< Light *> &lightList, 
                      const Light *ambient, const vector<Object *> &sceneList, 
                      double tooClose, int numShadowRays)
{
 vector3d_t randVec = vector3d_t(0,0,0);
 int count;
 rgb_t color, directColor;
 vector3d_t vectorToLight;
 ray_t ray;
 intercept_t interceptBeforelight;
 
 for(unsigned int l = 0; l < lightList.size(); l++)
 {
  count = 0;
  ray.orig = intercept.coord;
  directColor = lightList[l]->calculateLight(intercept);
  directColor = directColor * (1/(double)numShadowRays);
  do
  {
   if(count)
    randVec = vector3d_t(0.05 * (float)rand()/(float)RAND_MAX,
                         0.05 * (float)rand()/(float)RAND_MAX,
                         0.05 * (float)rand()/(float)RAND_MAX);
   vectorToLight = lightList[l]->getPosition() + randVec - ray.orig;
   ray.dir = normalize(vectorToLight);

   interceptBeforelight = kiran_find_intercept(ray, sceneList, tooClose,
                          norm(vectorToLight));

   if(interceptBeforelight.object == NULL) // light not occluded by objects 
    color = color + directColor;
   else
    color = color + (directColor * 0.5 * 
            interceptBeforelight.object->getTransmissionCoeff());
   count++;
  }while(count < numShadowRays);
  color = color + // ** add estimate from PHOTON MAP for indirect lights**
          (lightList[l])->d_photonMap.irradianceEstimate(intercept.coord, 
                          intercept.normal, 0.2, 1500);
 }
 
 // ambient lighting
 if( ambient != NULL )
  color = color + ambient->calculateLight(intercept);

 return color;
}


//==================================================================
// kiran_recursive_trace
//==================================================================
int kiran_recursive_trace(ray_t ray, const vector< Light *> &lightList,
                          const Light *ambient, const vector<Object *> &sceneList, 
                          double tooClose, double tooFar, int maxDepth,
                          int depth, rgb_t &color, int numShadowRays)
{
 int newDepth;
 ray_t newRay;
 intercept_t intercept;
 rgb_t localColor;
 rgb_t reflColor = color; // set init color to background
 rgb_t refrColor = color;
 double kr, kt;
 static double mui = 1;
 double mur; //refractive indices of incident and refracted rays
 double mr;
 newDepth = depth+1;

 intercept = kiran_find_intercept(ray, sceneList, tooClose, tooFar);
 if(intercept.object == NULL)
  return newDepth;

 // Direct illumination from light sources
 localColor = kiran_do_lights(intercept, lightList, ambient, sceneList, 
                              tooClose, numShadowRays);
  
 if(newDepth >= maxDepth)
 {
  reflColor = rgb_t(0,0,0);
  refrColor = rgb_t(0,0,0);
 }
 else
 {
  // Estimate contribution from reflections
  if(intercept.object->getRefCoeff() > 0)
  {
   newRay.orig = intercept.coord;
   newRay.dir = normalize(intercept.incidentRay - 2 * 
                           dot(intercept.incidentRay, intercept.normal) 
                           * intercept.normal);
   kiran_recursive_trace(newRay, lightList, ambient, sceneList, tooClose, tooFar, 
                           maxDepth, newDepth, reflColor, numShadowRays);
  }

  // Estimate contribution from refractions
  if(intercept.object->getTransmissionCoeff() > 0)
  {
   double iDotN, cosr;
   mur = intercept.object->getRefractiveIndex(); 
   iDotN = -1 * dot(intercept.incidentRay,intercept.normal);
   if(iDotN < 0)
   {
    intercept.normal = -1 * intercept.normal; // invert because we are inside object
    if(mur == mui) // going out of the object
     mur = 1;
   }
   newRay.orig = intercept.coord;
   mr = mui/mur;
   cosr = 1.0 + mr * ((iDotN * iDotN) - 1);
   if(cosr > 0)
   {
    newRay.dir = mr * intercept.incidentRay + intercept.normal
                 * (mr * fabs(iDotN) - sqrt(cosr));
    newRay.dir = normalize(newRay.dir);
    kiran_recursive_trace(newRay, lightList, ambient, sceneList, tooClose, tooFar, 
                                maxDepth, newDepth, refrColor, numShadowRays);
    //mui = mur;
   }
  }
 }
 kr = intercept.object->getRefCoeff();
 kt = intercept.object->getTransmissionCoeff();
 color = localColor + kr * reflColor + kt * refrColor;
 return newDepth;
}


//==================================================================
// kiran_trace
//==================================================================
rgb_t kiran_trace(ray_list_t *rayList, const vector< Light *> &lightList,
                  const Light *ambient, const vector<Object *> &sceneList, rgb_t bkColor,
                  double tooClose, double tooFar, int maxDepth, int depth, 
                  int numShadowRays)
{
 rgb_t color, tmpColor;
 tmpColor = bkColor;
 double numRays = 0;
 ray_list_t *tmpPtr;
 
 tmpPtr = rayList;
 while(tmpPtr != NULL)
 {
  numRays++;
  kiran_recursive_trace(tmpPtr->ray, lightList, ambient, sceneList, 
                          tooClose, tooFar, maxDepth, 
                          depth, tmpColor, numShadowRays);

  color.r = (1.0/numRays)*((numRays-1) * color.r + tmpColor.r);
  color.g = (1.0/numRays)*((numRays-1) * color.g + tmpColor.g);
  color.b = (1.0/numRays)*((numRays-1) * color.b + tmpColor.b);

  tmpPtr = tmpPtr->nxt; 
 }
 
 // release rays from memory
 while(rayList != NULL)
 {
  tmpPtr = rayList; 
  rayList = rayList->nxt;
  delete tmpPtr;
 }
 
 return color;
}


//==================================================================
// kiran_recursive_photon_trace
//==================================================================
int kiran_recursive_photon_trace(ray_t &ray, const vector<Object *> &sceneList,
                                  double tooClose, double tooFar, rgb_t &color)
{
 intercept_t intercept;
 int returnVal = -1;
 static float recursionDepth;
 static double mui = 1;
 double mur; //refractive indices of incident and refracted rays
 rgb_t objColor;

 if(recursionDepth > 5)
  return returnVal;
  
 intercept = kiran_find_intercept(ray, sceneList, tooClose, tooFar);
 if(intercept.object == NULL)
  return returnVal;
 
 recursionDepth++;

 // Change ray properties
 ray.orig = intercept.coord;
 ray.dir = intercept.incidentRay;
 objColor = intercept.object->getColor(intercept.coord);

 color.r = (1.0/recursionDepth) * ((recursionDepth-1) * color.r + ray.pow.r * objColor.r);
 color.g = (1.0/recursionDepth) * ((recursionDepth-1) * color.g + ray.pow.g * objColor.g);
 color.b = (1.0/recursionDepth) * ((recursionDepth-1) * color.b + ray.pow.b * objColor.b);


 float p, e;

 // specular reflections for caustics
 if( (p = intercept.object->getRefCoeff()) > 0)
 {
  e = (float)rand()/(float)RAND_MAX;
  if(e < p)
  {
   ray.dir = normalize(intercept.incidentRay - 2 * 
                       dot(intercept.incidentRay, intercept.normal) 
                       * intercept.normal);
   returnVal = kiran_recursive_photon_trace(ray, sceneList, tooClose, tooFar, color);
   recursionDepth = 0;
   return returnVal;
  }
 }

 // specular refractions for caustics
 if( (p = intercept.object->getTransmissionCoeff()) > 0)
 {
  e = (float)rand()/(float)RAND_MAX;
  if(e < p)
  {
   double iDotN, cosr, mr;
   mur = intercept.object->getRefractiveIndex(); 
   iDotN = -1 * dot(intercept.incidentRay,intercept.normal);
   if(iDotN < 0)
   {
    intercept.normal = -1 * intercept.normal; // invert because we are inside object
    if(mur == mui) // going out of the object
     mur = 1;
   }
   mr = mui/mur;
   cosr = 1.0 + mr * ((iDotN * iDotN) - 1);
   if(cosr > 0)
   {
    ray.dir = mr * intercept.incidentRay + intercept.normal
              * (mr * fabs(iDotN) - sqrt(cosr));
    ray.dir = normalize(ray.dir);
    returnVal = kiran_recursive_photon_trace(ray, sceneList, tooClose, tooFar, color);
    recursionDepth = 0;
    return returnVal;
   }
  }
 }


 // diffuse reflections and absorption
 if( (intercept.object->getRefCoeff() < 1) && (intercept.object->getTransmissionCoeff() < 1) )
 {
  e = (float)rand()/(float)RAND_MAX;
  if(e < 0.5)
  {
   ray.dir = intercept.incidentRay - 2 * 
                       dot(intercept.incidentRay, intercept.normal) 
                       * intercept.normal;
   ray.dir.x = 2.0 * ((float)rand()/(float)RAND_MAX - 0.5);
   ray.dir.y = 2.0 * ((float)rand()/(float)RAND_MAX - 0.5);
   ray.dir.z = 2.0 * ((float)rand()/(float)RAND_MAX - 0.5);
   ray.dir = normalize(ray.dir);

   returnVal = kiran_recursive_photon_trace(ray, sceneList, tooClose, tooFar, color);
   recursionDepth = 0;
   return returnVal;
  }
  recursionDepth = 0;
  return 0;
 } 

 recursionDepth = 0;
 return returnVal;
}


//==================================================================
// kiran_photon_trace - photon emission and storage
//==================================================================
void kiran_photon_trace(Light *light, int numPhotons, 
                        const vector<Object *> &sceneList, double tooClose, 
                        double tooFar)
{
 ray_t ray;
 rgb_t color;
 vector3d_t power;
 int i = 0; 
 float x,y,z;

 light->d_photonMap.init(numPhotons);

 while (i < numPhotons)
 {
  do
  {
   x = (float)rand()/(float)RAND_MAX - 0.5;
   y = (float)rand()/(float)RAND_MAX - 0.5;
   z = (float)rand()/(float)RAND_MAX - 0.5;
  }while(x*x + y*y + z*z > 0.25);

  // photon properties are set here
  ray.dir = vector3d_t(x,y,z);
  ray.dir = normalize(ray.dir);
  ray.orig = light->getPosition();
  ray.pow = light->getSourceIntensity();
  
  if(kiran_recursive_photon_trace(ray, sceneList, tooClose, tooFar, color) == 0)
   light->d_photonMap.store(color, ray.orig, ray.dir);
  i++;
 }
 light->d_photonMap.scalePhotonPower(1.0/(float)numPhotons);
 light->d_photonMap.balance();
}


//==================================================================
// main
//==================================================================
int main(int argc, char *argv[])
{
 int imageWidth = 640;  // default width
 int imageHeight = 480; // default height
 int numShadowRays = 1; // no. of rays from an intercept to light
 int numPhotons = 100;
 bool antiAlias = false;
 Pixmap *outputImage;   // output image
 char *outputFile = "output.ppm";
 char *inputFile = NULL;
 SceneReader sceneReader; // Scene file reader
 int maxDepth = 5;
  
//------------------------------------------------------------------
// Read command line options, initialize
//------------------------------------------------------------------
 int opt;
 while( (opt = getopt(argc, argv, "o:i:")) != -1)
 {
  switch(opt)
  {
   case 'o': // set output file name
    outputFile = optarg;
    break;
   case 'i': // set input scene file name
    inputFile = optarg;
    break;
   default:
    break;
   }
 }

 
 if (sceneReader.open(inputFile) != 0)
 {
  cerr << "kiran: ERROR opening input scene description file." << endl;
  exit(-1);
 }
 imageWidth = sceneReader.getImageWidth();
 imageHeight = sceneReader.getImageHeight();
 antiAlias = sceneReader.isAntiAliasEnabled();
 numShadowRays = sceneReader.getNumShadowRays();
 maxDepth = sceneReader.getRecursionDepth();
 numPhotons = sceneReader.getNumPhotons();
 if( (numPhotons != 0) && (numPhotons < 2000) ) numPhotons = 2000;

 sceneReader.printSceneInfo();
 cout << "Image size      : " << imageWidth << " x " << imageHeight << endl;
 cout << "Output file     : " << outputFile << endl;
 cout << "Shadow rays     : " << numShadowRays << " per intercept" << endl; 
 cout << "Recursion depth : " << maxDepth << endl; 
 cout << "Antialias       : "; 
         (antiAlias)?(cout << "enabled"):(cout << "disabled");
 cout << endl;
 cout << "Photons         : " << numPhotons << " per light source" << endl;
 

//------------------------------------------------------------------
// create scene list
//------------------------------------------------------------------
	
 vector<Object *> objectList; // list of all objects in scene
 vector<Light *> lightList;  // all lights in the scene, except ambient
 AmbientLight *aLight = NULL;
 Camera *camera = NULL;

 camera = sceneReader.getCamera();
 if(numPhotons == 0)
 {
  aLight = sceneReader.getAmbientLight();
  if(!aLight)
  {
   cerr << "kiran: ERROR reading ambient light properties" << endl;
   exit(-1);
  }
 }

 if(!camera)
 {
  cerr << "kiran: ERROR reading camera properties" << endl;
  exit(-1);
 }

 objectList = sceneReader.getObjectList();
 lightList = sceneReader.getLightList();


//------------------------------------------------------------------
// scan the scene
//------------------------------------------------------------------
 rgb_t color, color1, color2, color3, color4;
 double progress = 0, pfactor;
 ray_list_t *rayList;

 outputImage = new Pixmap(imageWidth, imageHeight);
 pfactor = 100.0/(imageWidth * imageHeight);
 
 // Do photon mapping from lights
 if(numPhotons != 0)
 {
  for(unsigned int i = 0; i < lightList.size(); i++)
  {
   kiran_photon_trace((lightList[i]), numPhotons, objectList, 
                      1e-6, camera->getFarClippingDistance());
  }
 }
 
 // Do ray tracing from eye/camera
 camera->setCcdSize(imageWidth, imageHeight);
 for(int u = 1; u <= imageWidth; u++)
 {
  for(int v = 1; v <= imageHeight; v++)
  {
   rayList = camera->getRays(u, v);
   color = sceneReader.getBackGroundColor(u,v);
   (*outputImage)(u, v) = kiran_trace(rayList, lightList, (Light *)(aLight),
                  objectList, color, 1e-6, camera->getFarClippingDistance(), 
                  maxDepth, 0, numShadowRays);

   // Super-sampling for anti-aliasing
   if(antiAlias)
   {
    if(u == 1)
    {
     color1 = color2 = color3 = color4 = sceneReader.getBackGroundColor(u,v);

     rayList = camera->getRays(u-0.5, v-0.5);
     color1 = kiran_trace(rayList, lightList, (Light *)(aLight),
                  objectList, color1, 1e-6, camera->getFarClippingDistance(), 
                  maxDepth, 0, numShadowRays);
                  
     rayList = camera->getRays(u-0.5, v+0.5);
     color2 = kiran_trace(rayList, lightList, (Light *)(aLight),
                  objectList, color2, 1e-6, camera->getFarClippingDistance(), 
                  maxDepth, 0, numShadowRays);

     rayList = camera->getRays(u+0.5, v+0.5);
     color3 = kiran_trace(rayList, lightList, (Light *)(aLight),
                  objectList, color3, 1e-6, camera->getFarClippingDistance(), 
                  maxDepth, 0, numShadowRays);

     rayList = camera->getRays(u+0.5, v-0.5);
     color4 = kiran_trace(rayList, lightList, (Light *)(aLight),
                  objectList, color4, 1e-6, camera->getFarClippingDistance(), 
                  maxDepth, 0, numShadowRays);
    }
    else
    {
     color1 = color4;
     color2 = color3;
     color3 = color4 = sceneReader.getBackGroundColor(u,v);

     rayList = camera->getRays(u+0.5, v+0.5);
     color3 = kiran_trace(rayList, lightList, (Light *)(aLight),
                  objectList, color3, 1e-6, camera->getFarClippingDistance(), 
                  maxDepth, 0, numShadowRays);

     rayList = camera->getRays(u+0.5, v-0.5);
     color4 = kiran_trace(rayList, lightList, (Light *)(aLight),
                  objectList, color4, 1e-6, camera->getFarClippingDistance(), 
                  maxDepth, 0, numShadowRays);
    }
    (*outputImage)(u, v) = 0.5 * (*outputImage)(u, v) + 0.125 * color1 + 
                           0.125 * color2 + 0.125 * color3 + 0.125 * color4; 
   }
   progress++;
   cout << "\rRendering       : " << (ceil)(progress * pfactor) << " % done";
  }
 }
 cout << endl << flush;
//------------------------------------------------------------------
// write output, clean up and exit
//------------------------------------------------------------------
	
 outputImage->write(outputFile, P6);
 delete outputImage;

 return 0;
}
