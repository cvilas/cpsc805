//==================================================================
// SceneReader.hpp  Scene file handling routines - Reads a scene
//                  description file and creates a vector list of 
//                  all objects in the scene.
//
// Author:          Vilas Kumar Chitrakaran (cvilas@ces.clemson.edu)
//                  Feb 2004
//==================================================================

#ifndef _SCENEREADER_HPP_INCLUDED
#define _SCENEREADER_HPP_INCLUDED

#include <fstream>
#include <vector>
#include <strstream>
#include "objects.hpp"
#include "Pixmap.hpp"
#include "data_types.hpp"

//==================================================================
// struct _section  A structure for beginning and end of a section
//                  in a scene file.
//==================================================================
typedef struct _section
{
 char name[80];
 int begin; // pointer to beginning of section
 int end;   // pointer to end of section
}section_t;


//==================================================================
// class SceneReader
//==================================================================
class SceneReader
{
 public:
  SceneReader(char *sceneFile = NULL);
   // The constructor. Reads the scene
   // description file, creates lists.
   
  ~SceneReader();
   // The destructor cleans up.
   
  int open(char *sceneFile);
   // Open scene file.
   //  sceneFile  File containing description of the scene
  
  Camera *getCamera();
   //  return  Pointer to camera object,
   //          or NULL on error or when no camera specified
   
  vector <Object *> getObjectList();
   //  return  List of all objects in the scene
   //          excluding lights
   
  vector <Light *> getLightList();
   //  return  List of all lights in the scene.
   //          This list does not include ambient
   //          light.
 
  AmbientLight *getAmbientLight();
   //  return  Pointer to ambient light.
   //          or NULL on error or when no ambient light specified
   
  rgb_t getBackGroundColor(int column, int row); 
   //  return  Scene background color. 
   //          Returns "black" if no background color
   //          or background image specified.
 
 int getImageWidth();
  //  return  Width of rendered output image
  
 int getImageHeight();
  //  return  Height of rendered output image

 bool isAntiAliasEnabled();
  //  return  True if anti-aliasing is on

 int getNumShadowRays();
  // Number of shadow rays to create soft shadows
   
 int getRecursionDepth();
  // Depth of recursion in tracing reflected and
  // refracted rays
  
 int getNumPhotons();
  // Number of photons to use for photon mapping. Specify 
  // 0 to disable photon mapping
  
 void printSceneInfo();
  // Print information about the scene to the standard 
  // output. This includes information about lights, 
  // objects and camera.

 private:
  void getScalarRecord(section_t *section, char *name, double &value, double def);
  void getVectorRecord(section_t *section, char *name, vector3d_t &value, vector3d_t def);
  void getStringRecord(section_t *section, char *name, char *value, char *def);
  void readCommonProperties(section_t *section, Object *object);
  Object *readSphere(section_t *section);
  Object *readInfinitePlane(section_t *section);
  Object *readCheckerBoard(section_t *section);
  Object *readPlanarConvexQuad(section_t *section);
  Object *readBox(section_t *section);
  Object *readZCylinder(section_t *section);
  AmbientLight *readAmbientLight(section_t *section);
  Light *readPointLight(section_t *section);
  Camera *readCamera(section_t *section);
  void readBackGround(section_t *section);
  void readGlobalSettings(section_t *section);
  
  fstream d_file;
  Camera *d_camera;
  vector<Object *> d_objectList;
  vector<Light *> d_lightList;
  vector<section_t *> d_sectionList;
  AmbientLight *d_ambientLight;
  Pixmap *d_bkImage;
  rgb_t d_bkColor;
  bool d_bkImageSpecified;
  int d_imageWidth;
  int d_imageHeight;
  bool d_isAntiAliasEnabled;
  int d_recursionDepth;
  int d_numPhotons;
  int d_numShadowRays;
};

#endif // ifndef _SCENEREADER_HPP_INCLUDED
