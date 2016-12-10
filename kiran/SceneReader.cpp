#include "SceneReader.hpp"
#include <typeinfo>

//==================================================================
// SceneReader::SceneReader
//==================================================================
SceneReader::SceneReader( char *sceneFile )
{
 d_objectList.clear();
 d_lightList.clear();
 d_camera = NULL;
 d_ambientLight = NULL;
 d_bkImage = NULL;
 d_bkImageSpecified = false;
 d_bkColor = rgb_t(0,0,0);
 d_imageWidth = 640;
 d_imageHeight = 480;
 d_isAntiAliasEnabled = false;
 d_numShadowRays = 1;
 
 if(sceneFile == NULL)
  return;
 
 if( open(sceneFile) != 0 )
 {
  cerr << "SceneReader: ERROR opening file " << sceneFile << endl;
  exit(EXIT_FAILURE);
 }
}


//==================================================================
// SceneReader::~SceneReader
//==================================================================
SceneReader::~SceneReader()
{
 if(d_objectList.size())
 {
  for(unsigned int i = 0; i < d_objectList.size(); i++)
   delete d_objectList[i];
 }

 if(d_lightList.size())
 {
  for(unsigned int i = 0; i < d_lightList.size(); i++)
   delete d_lightList[i];
 }
 
 if(d_camera)
  delete d_camera;
 
 if(d_ambientLight)
  delete d_ambientLight;
 
 if(d_bkImage)
  delete d_bkImage;
  
 d_file.close();
}


//==================================================================
// SceneReader::open
//==================================================================
int SceneReader::open(char *sceneFile)
{
 if(sceneFile == NULL)
  return(-1);

 d_file.open(sceneFile, fstream::in);
 if( !d_file.is_open() )
  return(-1);
 
 cout << "SceneReader: Processing scene file " << sceneFile << "." 
      << endl;
 
 Object *object;
 Light *light;
 int fileLen;
 char buf[80];
 
 // get length of scene file
 d_file.seekg(0, ios::end);
 fileLen = d_file.tellg();
 d_file.seekg(0, ios::beg);
 
 // Make a list of all sections
 d_sectionList.clear();
 while(!d_file.eof())
 {
  d_file.ignore(fileLen,'<'); // begin class section
  d_file.getline(buf, 80,'>'); // get class name
  section_t *section = new section_t;
  section->begin = d_file.tellg();
  strcpy(section->name, buf);
  d_file.ignore(fileLen, '<');
  if(d_file.peek() == '/') // </ is end class
  {
   section->end = d_file.tellg();
   d_sectionList.push_back(section);
   d_file.ignore(fileLen, '\n'); // ignore rest of the line
  }
 }


 // Step through sections
 for(unsigned int i = 0; i < d_sectionList.size(); i++)
 {
  if(strstr(d_sectionList[i]->name, "Sphere") != NULL)
  {
   object = readSphere(d_sectionList[i]);
   d_objectList.push_back(object);
   continue;
  }

  if(strstr(d_sectionList[i]->name, "InfinitePlane") != NULL)
  {
   object = readInfinitePlane(d_sectionList[i]);
   d_objectList.push_back(object);
   continue;
  }

  if(strstr(d_sectionList[i]->name, "CheckerBoard") != NULL)
  {
   object = readCheckerBoard(d_sectionList[i]);
   d_objectList.push_back(object);
   continue;
  }

  if(strstr(d_sectionList[i]->name, "PlanarConvexQuad") != NULL)
  {
   object = readPlanarConvexQuad(d_sectionList[i]);
   d_objectList.push_back(object);
   continue;
  }

  if(strstr(d_sectionList[i]->name, "Box") != NULL)
  {
   object = readBox(d_sectionList[i]);
   d_objectList.push_back(object);
   continue;
  }

  if(strstr(d_sectionList[i]->name, "ZCylinder") != NULL)
  {
   object = readZCylinder(d_sectionList[i]);
   d_objectList.push_back(object);
   continue;
  }

  if(strstr(d_sectionList[i]->name, "PointLight") != NULL)
  {
   light = readPointLight(d_sectionList[i]);
   d_lightList.push_back(light);
   continue;
  }

  if(strstr(d_sectionList[i]->name, "AmbientLight") != NULL)
  {
   d_ambientLight = readAmbientLight(d_sectionList[i]);
   continue;
  }

  if(strstr(d_sectionList[i]->name, "Camera") != NULL)
  {
   d_camera = readCamera(d_sectionList[i]);
   continue;
  }

  if(strstr(d_sectionList[i]->name, "Background") != NULL)
  {
   readBackGround(d_sectionList[i]);
   continue;
  }

  if(strstr(d_sectionList[i]->name, "Global") != NULL)
  {
   readGlobalSettings(d_sectionList[i]);
   continue;
  }
  
  cout << "SceneReader: ERROR unknown object " << d_sectionList[i]->name 
       << endl;
 }
 d_file.close();
 return 0;
}


//==================================================================
// SceneReader::getCamera
//==================================================================
Camera *SceneReader::getCamera()
{
 if(d_camera)
  return d_camera;
 return NULL;
}


//==================================================================
// SceneReader::getObjectList
//==================================================================
vector <Object *> SceneReader::getObjectList()
{
 return d_objectList;
}


//==================================================================
// SceneReader::getLightList
//==================================================================
vector <Light *> SceneReader::getLightList()
{
 return d_lightList;
}


//==================================================================
// SceneReader::getAmbientLight
//==================================================================
AmbientLight *SceneReader::getAmbientLight()
{
 if(d_ambientLight)
  return d_ambientLight;
 return NULL;
}


//==================================================================
// SceneReader::getBackGroundColor
//==================================================================
rgb_t SceneReader::getBackGroundColor(int col, int row)
{
 if(!d_bkImageSpecified)
  return d_bkColor;
 
 if(col > d_bkImage->width() || row > d_bkImage->height())
  return rgb_t(0,0,0);
 
 return (*d_bkImage)(col, row);
}


//==================================================================
// SceneReader::getImageWidth
//==================================================================
int SceneReader::getImageWidth()
{
 return d_imageWidth;
}
  

//==================================================================
// SceneReader::getImageHeight
//==================================================================
int SceneReader::getImageHeight()
{
 return d_imageHeight;
}


//==================================================================
// SceneReader::isAntiAliasEnabled
//==================================================================
bool SceneReader::isAntiAliasEnabled()
{
 return d_isAntiAliasEnabled;
}


//==================================================================
// SceneReader::getNumShadowRays
//==================================================================
int SceneReader::getNumShadowRays()
{
 return d_numShadowRays;
}


//==================================================================
// SceneReader::printSceneInfo
//==================================================================
void SceneReader::printSceneInfo()
{
 cout << "SceneReader: list of objects" << endl;
 for(unsigned int i = 0; i < d_objectList.size(); i++)
  cout << "  --> " << d_objectList[i]->getName() << endl;

 cout << "SceneReader: list of lights" << endl;
 for(unsigned int i = 0; i < d_lightList.size(); i++)
  cout << "  --> " << d_lightList[i]->getName() << endl;
 
 if(d_ambientLight)
  cout << "  --> " << d_ambientLight->getName() << endl;
}


//==================================================================
// SceneReader::getScalarRecord
//==================================================================
void SceneReader::getScalarRecord(section_t *section, char *name, 
                                  double &value, double def)
{
 char buf[80];
 int nameLen = strlen(name);
 int i = section->begin;
 value = def;
 d_file.clear();  // clear eof or bad state
 d_file.seekg(i, ios::beg);
 while(i < section->end)
 {
  d_file.getline(buf, 80);
  if(strncmp(buf, name, nameLen) == 0) // check for name matches
  {
   // make sure match is not just in part
   if(isalpha(buf[nameLen]) || isdigit(buf[nameLen])) 
    continue;
   istrstream data((buf+nameLen+1));
   data >> value;
  }
  i = d_file.tellg();
 }
}


//==================================================================
// SceneReader::getVectorRecord
//==================================================================
void SceneReader::getVectorRecord(section_t *section, char *name, 
                                 vector3d_t &value, vector3d_t def)
{
 char buf[80];
 int nameLen = strlen(name);
 int i = section->begin;
 value = def;
 d_file.clear();  // clear eof or bad state
 d_file.seekg(i, ios::beg);
 while(i < section->end)
 {
  d_file.getline(buf, 80);
  if(strncmp(buf, name, nameLen) == 0) // check for name matches
  {
   // make sure match is not just in part
   if(isalpha(buf[nameLen]) || isdigit(buf[nameLen])) 
    continue;
   istrstream data((buf+nameLen+1));
   data >> value.x;
   data >> value.y;
   data >> value.z;
  }
  i = d_file.tellg();
 }
}


//==================================================================
// SceneReader::getStringRecord
//==================================================================
void SceneReader::getStringRecord(section_t *section, char *name, 
                                          char *value, char *def)
{
 char buf[80];
 int nameLen = strlen(name);
 int i = section->begin;
 strcpy(value, def);
 d_file.clear();  // clear eof or bad state
 d_file.seekg(i, ios::beg);
 while(i < section->end)
 {
  d_file.getline(buf, 80);
  if(strncmp(buf, name, nameLen) == 0) // check for name matches
  {
   // make sure match is not just in part
   if(isalpha(buf[nameLen]) || isdigit(buf[nameLen])) 
    continue;
   strcpy(value, (buf+nameLen+1));
  }
  i = d_file.tellg();
 }
}


//==================================================================
// SceneReader::readCommonProperties
//==================================================================
void SceneReader::readCommonProperties(section_t *section, Object *object)
{
 double sc;
 vector3d_t vec;
 char str[80];
 rgb_t color;
 
 getStringRecord(section, "name", str, "noname");
 object->setName(str);

 getVectorRecord(section, "color", vec, vector3d_t(0,0,0));
 color.r = vec.x;
 color.g = vec.y;
 color.b = vec.z;
 object->setColor(color);

 getStringRecord(section, "texture", str, "NULL");
 if( strcmp(str, "NULL") != 0 )
  object->setTexture(str);

 getStringRecord(section, "bump_map", str, "NULL");
 if( strcmp(str, "NULL") != 0 )
  object->setBumpMap(str);
 
 getScalarRecord(section, "bumpiness", sc, 0);
 object->setBumpiness(sc);

 getScalarRecord(section, "diffuse", sc, 0);
 object->setDiffuseLtCoeff(sc);

 getScalarRecord(section, "ambient", sc, 0);
 object->setAmbLtCoeff(sc);

 getScalarRecord(section, "phong", sc, 0);
 object->setSpecLtCoeff(sc);

 getScalarRecord(section, "phong_size", sc, 0);
 object->setSpecLtExp((int)sc);

 getScalarRecord(section, "reflectivity", sc, 0);
 object->setReflectivity(sc);

 getScalarRecord(section, "transmittivity", sc, 0);
 object->setTransmittivity(sc);

 getScalarRecord(section, "refractive_index", sc, 1);
 object->setRefractiveIndex(sc);

 getVectorRecord(section, "translate", vec, vector3d_t(0,0,0));
 object->translate(vec.x, vec.y, vec.z);

 getVectorRecord(section, "rotate", vec, vector3d_t(0,0,0));
 object->rotate(vec.x, vec.y, vec.z);
}


//==================================================================
// SceneReader::readSphere
//==================================================================
Object *SceneReader::readSphere(section_t *section)
{
 double sc;
 vector3d_t vec;

 Object *object = new Sphere;
 readCommonProperties(section, object);

 getScalarRecord(section, "radius", sc, 0);
 ((Sphere *)object)->setRadius(sc);

 return object;
}


//==================================================================
// SceneReader::readInfinitePlane
//==================================================================
Object *SceneReader::readInfinitePlane(section_t *section)
{
 vector3d_t vec;

 Object *object = new InfinitePlane;
 readCommonProperties(section, object);

 return object;
}


//==================================================================
// SceneReader::readCheckerBoard
//==================================================================
Object *SceneReader::readCheckerBoard(section_t *section)
{
 double sc;
 vector3d_t vec;
 rgb_t color;

 Object *object = new CheckerBoard;
 readCommonProperties(section, object);

 getVectorRecord(section, "color2", vec, vector3d_t(0,0,0));
 color.r = vec.x;
 color.g = vec.y;
 color.b = vec.z;
 ((CheckerBoard *)object)->setColor2(color);

 getScalarRecord(section, "check_size", sc, 0);
 ((CheckerBoard *)object)->setCheckerSize(sc);

 return object;
}


//==================================================================
// SceneReader::readPlanarConvexQuad
//==================================================================
Object *SceneReader::readPlanarConvexQuad(section_t *section)
{
 vector3d_t v1, v2, v3, v4;
 Object *object = new PlanarConvexQuad;
 
 readCommonProperties(section, object);
 
 getVectorRecord(section, "vertex1", v1, vector3d_t(0,0,0));
 getVectorRecord(section, "vertex2", v2, vector3d_t(0,0,0));
 getVectorRecord(section, "vertex3", v3, vector3d_t(0,0,0));
 getVectorRecord(section, "vertex4", v4, vector3d_t(0,0,0));
 ((PlanarConvexQuad *)object)->setVertices(v1, v2, v3, v4);

 return object;
}


//==================================================================
// SceneReader::readBox
//==================================================================
Object *SceneReader::readBox(section_t *section)
{
 vector3d_t v1, v2;
 Object *object = new Box;
 
 readCommonProperties(section, object);
 getVectorRecord(section, "lo", v1, v1);
 getVectorRecord(section, "hi", v2, v2);
 ((Box *)object)->setVertices(v1, v2);

 return object;
}


//==================================================================
// SceneReader::readZCylinder
//==================================================================
Object *SceneReader::readZCylinder(section_t *section)
{
 vector3d_t v;
 double sc;
 char str[80];

 Object *object = new ZCylinder;

 readCommonProperties(section, object);
 getVectorRecord(section, "position", v, vector3d_t(0,0,0));
 ((ZCylinder *)object)->setPosition(v);
 getScalarRecord(section, "radius", sc, 0);
 ((ZCylinder *)object)->setRadius(sc);
 getScalarRecord(section, "length", sc, 0);
 ((ZCylinder *)object)->setLength(sc);
 getStringRecord(section, "show_end_caps", str, "no");
 if(strcmp(str, "yes") == 0)
  ((ZCylinder *)object)->setEndCapsOn();

 return object;
}


//==================================================================
// SceneReader::readPointLight
//==================================================================
Light *SceneReader::readPointLight(section_t *section)
{
 Light *light = new PointLight;
 vector3d_t vec;
 char str[80];
 
 getStringRecord(section, "name", str, "noname");
 ((PointLight *)light)->setName(str);

 getVectorRecord(section, "position", vec, vector3d_t(0,0,0));
 ((PointLight *)light)->setPosition(vec);
 
 getVectorRecord(section, "intensity", vec, vector3d_t(0,0,0));
 ((PointLight *)light)->setIntensity(vec.x, vec.y, vec.z);

 getVectorRecord(section, "attenuation", vec, vector3d_t(0,0,0));
 ((PointLight *)light)->setIntensityAttnFactors(vec.x, vec.y, vec.z);

 return light;
}


//==================================================================
// SceneReader::readAmbientLight
//==================================================================
AmbientLight *SceneReader::readAmbientLight(section_t *section)
{
 AmbientLight *light = new AmbientLight;
 vector3d_t vec;
 char str[80];
 
 getStringRecord(section, "name", str, "noname");
 ((AmbientLight *)light)->setName(str);

 getVectorRecord(section, "intensity", vec, vector3d_t(0,0,0));
 ((AmbientLight *)light)->setIntensity(vec.x, vec.y, vec.z);

 return light;
}


//==================================================================
// SceneReader::readBackGround
//==================================================================
void SceneReader::readBackGround(section_t *section)
{
 vector3d_t vec;
 rgb_t color;
 char str[80];
 
 getStringRecord(section, "image", str, "noname");
 if(strcmp(str, "noname") != 0)
 {
  d_bkImageSpecified = true;
  d_bkImage = new Pixmap;
  d_bkImage->open(str);
 }
 getVectorRecord(section, "color", vec, vector3d_t(0,0,0));
 color.r = vec.x;
 color.g = vec.y;
 color.b = vec.z;
 d_bkColor = color;
}


//==================================================================
// SceneReader::readGlobalSettings
//==================================================================
void SceneReader::readGlobalSettings(section_t *section)
{
 char str[80];
 double sc;

 getScalarRecord(section, "num_shadow_rays", sc, 1);
  d_numShadowRays = (int)sc;

 getScalarRecord(section, "image_width", sc, 320);
  d_imageWidth = (int)sc;

 getScalarRecord(section, "image_height", sc, 240);
  d_imageHeight = (int)sc;

 getStringRecord(section, "anti_alias", str, "no");
 if(strcmp(str, "yes") == 0)
  d_isAntiAliasEnabled = true;
}


//==================================================================
// SceneReader::readCamera
//==================================================================
Camera *SceneReader::readCamera(section_t *section)
{
 double sc;
 vector3d_t vec1, vec2, vec3;

 Camera *camera = new Camera;

 getScalarRecord(section, "focal_length", sc, 0);
 camera->setLensFocalLength(sc);

 getScalarRecord(section, "focus", sc, 0);
 camera->setFocalDistance(sc);

 getScalarRecord(section, "far_clipping_distance", sc, 10);
 camera->setFarClippingDistance(sc);

 getScalarRecord(section, "f_stop", sc, 32);
 camera->setAperture(sc);

 getVectorRecord(section, "position", vec1, vector3d_t(0,0,0));
 getVectorRecord(section, "look_at", vec2, vector3d_t(0,0,1));
 getVectorRecord(section, "up", vec3, vector3d_t(1,0,0));
 camera->setPosition(vec1, vec2, vec3);
 
 return camera;
}



