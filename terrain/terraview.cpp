//==================================================================
// terraview.cpp   Mountain terrain viewer version 0.1
//
// Author:         Vilas Kumar Chitrakaran (cvilas@ces.clemson.edu)
//                 April 15, 2004
//==================================================================

#include "data_types.hpp"
#include "imageReader.h"
#include <GL/glut.h>
#include <stdlib.h>
#include <math.h>

using namespace std;

typedef struct _terraview_global
{
 bool reset;
 bool activeMouse;
 double azimuth;
 double elevation;
 int zoom;
 vector3d_t eye_pos;
 vector3d_t eye_lookAt;
 vector3d_t eye_up;
 GLuint width;
 GLuint height;
 GLuint window_id;
 double roughness;
 int seed;
 int depth;
}terraview_global_t;


static terraview_global_t terraviewGlobal;

//==================================================================
// window callbacks
//==================================================================
static void createSceneViewer(int xPosition, int yPosition, int width, int height);
static void updateScene();
static void displayCallback();
static void keyboardCallback(unsigned char key, int mouseX, int mouseY);
static void reshapeCallback(int width, int height);
static void motionCallback(int mouseX, int mouseY);
static void mouseCallback(int button, int buttonState, int mouseX, int mouseY);
static void idleCallback();
static void reset_eye();

//==================================================================
// terrain generation functions
//==================================================================
int vertexGen(float *varray, int n, float rough, int randseed);
void getColor(float height, float *color);
void drawTerrain();

//==================================================================
// drawTerrain
//==================================================================
void drawTerrain()
{
 float *varray;
 vector3d_t v12, v24, v43, v31, vertex[4], normal[4];
 float color[4];
 float height[4];
 int row, col;
 double extent = 50;
 double x, z;
 double numSteps = pow(2.0, terraviewGlobal.depth);
 int numCols = (int)numSteps + 1;
 double step;
 GLuint texture;
 ImageForGl textureImage;
 
 int numElements = (int)((numSteps + 1) * (numSteps + 1));
 varray = (float *)calloc(numElements, sizeof(float)); 
 vertexGen(varray, terraviewGlobal.depth, terraviewGlobal.roughness,
           terraviewGlobal.seed);
 step = 2 * extent/numSteps;
 color[3] = 1.0;

 glNewList(1, GL_COMPILE_AND_EXECUTE);
  // mountain terrain
  x = -extent; 
  z = -extent;
  for(row = 0; row < numSteps - 1; row++)
  {
   glBegin(GL_QUAD_STRIP);
    x = -extent;
    for(col = 0; col < numSteps - 1; col++)
    { 
     // Generate vertex coords
     height[0] = varray[row * numCols + col];
     vertex[0] = vector3d_t(x, 0.5 * extent * height[0], z);
     height[1] = varray[(row+1) * numCols + col];
     vertex[1] = vector3d_t(x, 0.5 * extent * height[1], z+step);
     col++;
     x+=step;
     height[2] = varray[row * numCols + col];
     vertex[2] = vector3d_t(x, 0.5 * extent * height[2], z);
     height[3] = varray[(row+1) * numCols + col];
     vertex[3] = vector3d_t(x, 0.5 * extent * height[3], z+step);
     x+=step;
     
     // Generate vertex normals
     v12 = vertex[1] - vertex[0];
     v24 = vertex[3] - vertex[1];
     v43 = vertex[2] - vertex[3];
     v31 = vertex[0] - vertex[2];
     normal[0] = cross(v12, (-1.0 * v31));
     normal[1] = cross(v24, (-1.0 * v12));
     normal[2] = cross(v31, (-1.0 * v43));
     normal[3] = cross(v43, (-1.0 * v24));
     
     // Specify vertex and normal coords
     for (int v = 0; v < 4; v++)
     {
      getColor(1.3 * height[v], color);
      glColor4fv(color);
      glNormal3f(normal[v].x, normal[v].y, normal[v].z);
      glVertex3f(vertex[v].x, vertex[v].y, vertex[v].z);
     }
    }
    z+=step;
   glEnd();
  } 

  glEnable(GL_TEXTURE_2D);

  // Water surface
  extent *= 2;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  textureImage = readPPMImage("ppms/lo.ppm");
  glTexImage2D( GL_TEXTURE_2D, 0, 3,
   	 	textureImage.numColumns, textureImage.numRows, 0,
 		GL_RGB, GL_UNSIGNED_BYTE, textureImage.imageData );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  glBegin(GL_QUADS);
   glColor4f(0.92,0.94,0.96,1.0);
   glTexCoord2f(0,1);
   glVertex3f(-extent, 0.3, extent);
   glTexCoord2f(0,0);
   glVertex3f(extent, 0.3, extent);
   glTexCoord2f(1,0);
   glVertex3f(extent, 0.3, -extent);
   glTexCoord2f(1,1);
   glVertex3f(-extent, 0.3, -extent);
  glEnd();
  glDeleteTextures(1, &texture);
  freeImageMemory(&textureImage);

  // front sky surface
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  textureImage = readPPMImage("ppms/ft.ppm");
  glTexImage2D( GL_TEXTURE_2D, 0, 3,
   	 	textureImage.numColumns, textureImage.numRows, 0,
 		GL_RGB, GL_UNSIGNED_BYTE, textureImage.imageData );

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_REPLACE);

  glBegin(GL_QUADS);
   glColor4f(0.92,0.94,0.96,1.0);
   glTexCoord2f(1,1);
   glVertex3f(-extent, 0.3, -extent);
   glTexCoord2f(0,1);
   glVertex3f(extent, 0.3, -extent);
   glColor4f(0.5,0.553,0.968,1.0);
   glTexCoord2f(0,0);
   glVertex3f(extent, extent * 0.5, -extent);
   glTexCoord2f(1,0);
   glVertex3f(-extent, extent * 0.5, -extent);
  glEnd();
  glDeleteTextures(1, &texture);
  freeImageMemory(&textureImage);
  
  // back sky surface
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  textureImage = readPPMImage("ppms/bk.ppm");
  glTexImage2D( GL_TEXTURE_2D, 0, 3,
   	 	textureImage.numColumns, textureImage.numRows, 0,
 		GL_RGB, GL_UNSIGNED_BYTE, textureImage.imageData );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_REPLACE);

  glBegin(GL_QUADS);
   glColor4f(0.92,0.94,0.96,1.0);
   glTexCoord2f(1,1);
   glVertex3f(extent, 0.3, extent);
   glTexCoord2f(0,1);
   glVertex3f(-extent, 0.3, extent);
   glColor4f(0.5,0.553,0.968,1.0);
   glTexCoord2f(0,0);
   glVertex3f(-extent, extent * 0.5, extent);
   glTexCoord2f(1,0);
   glVertex3f(extent, extent * 0.5, extent);
  glEnd();
  glDeleteTextures(1, &texture);
  freeImageMemory(&textureImage);
  
  // right sky surface
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  textureImage = readPPMImage("ppms/rt.ppm");
  glTexImage2D( GL_TEXTURE_2D, 0, 3,
   	 	textureImage.numColumns, textureImage.numRows, 0,
 		GL_RGB, GL_UNSIGNED_BYTE, textureImage.imageData );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_REPLACE);

  glBegin(GL_QUADS);
   glColor4f(0.92,0.94,0.96,1.0);
   glTexCoord2f(1,1);
   glVertex3f(extent, 0.3, -extent);
   glTexCoord2f(0,1);
   glVertex3f(extent, 0.3, extent);
   glColor4f(0.5,0.553,0.968,1.0);
   glTexCoord2f(0,0);
   glVertex3f(extent, extent * 0.5, extent);
   glTexCoord2f(1,0);
   glVertex3f(extent, extent * 0.5, -extent);
  glEnd();
  glDeleteTextures(1, &texture);
  freeImageMemory(&textureImage);

  // left sky surface
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  textureImage = readPPMImage("ppms/lf.ppm");
  glTexImage2D( GL_TEXTURE_2D, 0, 3,
   	 	textureImage.numColumns, textureImage.numRows, 0,
 		GL_RGB, GL_UNSIGNED_BYTE, textureImage.imageData );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_REPLACE);

  glBegin(GL_QUADS);
   glColor4f(0.92,0.94,0.96,1.0);
   glTexCoord2f(1,1);
   glVertex3f(-extent, 0.3, extent);
   glTexCoord2f(0,1);
   glVertex3f(-extent, 0.3, -extent);
   glColor4f(0.5,0.553,0.968,1.0);
   glTexCoord2f(0,0);
   glVertex3f(-extent, extent * 0.5, -extent);
   glTexCoord2f(1,0);
   glVertex3f(-extent, extent * 0.5, extent);
  glEnd();
  glDeleteTextures(1, &texture);
  freeImageMemory(&textureImage);
  
  // top sky surface
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  textureImage = readPPMImage("ppms/tp.ppm");
  glTexImage2D( GL_TEXTURE_2D, 0, 3,
   	 	textureImage.numColumns, textureImage.numRows, 0,
 		GL_RGB, GL_UNSIGNED_BYTE, textureImage.imageData );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_REPLACE);

  glBegin(GL_QUADS);
   glColor4f(0.5,0.553,0.968,1.0);
   glTexCoord2f(0,0);
   glVertex3f(-extent, extent * 0.5, extent);
   glTexCoord2f(1,0);
   glVertex3f(-extent, extent * 0.5, -extent);
   glTexCoord2f(1,1);
   glVertex3f(extent, extent * 0.5, -extent);
   glTexCoord2f(0,1);
   glVertex3f(extent, extent * 0.5, extent);
  glEnd();
  glDeleteTextures(1, &texture);
  freeImageMemory(&textureImage);
  glDisable(GL_TEXTURE_2D);

 glEndList();
 printf("Terrain vertices generated\n");
 free(varray);
}


//==================================================================
// vertexGen
//==================================================================
int vertexGen(float *varray, int n, 
              float rough, int randseed)
{
 // varray: 2D vertex array of size (2^n + 1) * (2^n + 1) 
 // n: number of subdivisions = 2^n + 1;
 // rough: roughness [0.0 - 1.0]
 
 int rmax, cmax, numcols;
 int rstep, cstep;
 float prange;      // range of perturbation
 int i,r,c;
 
 rmax = cmax = (int)pow(2.0,n);
 numcols = cmax + 1;
 
 srand(randseed);
 
 varray[0] = 0;
 varray[cmax] = 0;
 varray[rmax * numcols] = 0;
 varray[rmax * numcols + cmax] = 0;
 
 for(i = 0; i < n; i++)
 {
  // square size is (r,c) to (r + rstep, c + cstep)
  // for this pass 
  rstep = (int)(rmax/pow(2.0,i));
  cstep = (int)(cmax/pow(2.0,i));	

  prange = pow(rough, i);
  
  for(r = 0; r <= (rmax - rstep); r += rstep)
  {
   for(c = 0; c <= (cmax - cstep); c+= cstep)
   {

    //---- SQUARE step
    int rc, cc;      // center of square
    float perturb;  // perturbation
   
    rc = r + rstep/2;
    cc = c + cstep/2;

    perturb = 2 * prange * (float)rand()/(float)RAND_MAX - prange;

    varray[rc * numcols + cc] = 0.25 * (varray[r * numcols + c] 
                             + varray[r * numcols + (c+cstep)]
                             + varray[(r+rstep) * numcols + c] 
                             + varray[(r+rstep) * numcols + (c+cstep)]) 
                     + perturb;
   
    //---- DIAMOND step
    float left, bottom, top, right;
    float ilocal;
     
    // (rc,c) coordinate
    ilocal = 0;
    if(cc - cstep >= 0)
    {
     left = varray[rc * numcols + (cc - cstep)];
     ilocal++;
    } 
    else
     left = 0;
    
    right = varray[rc * numcols + cc];
    ilocal++;
    
    top = varray[r * numcols + c];
    ilocal++;
    
    bottom = varray[(r+rstep) * numcols + c];
    ilocal++;
    
    if(c == 0)
     varray[rc * numcols + c] = 0;
    else
    {
     perturb = 2 * prange * (float)rand()/(float)RAND_MAX - prange;
     varray[rc * numcols + c] = (top + bottom + left + right)/ilocal 
                               + perturb;
    }

    // (r+rstep,cc) coordinate
    ilocal = 0;
    
    left = varray[(r+rstep) * numcols + c];
    ilocal++;
    
    right = varray[(r+rstep) * numcols + (c+cstep)];
    ilocal++;
    
    top = varray[rc * numcols + cc];
    ilocal++;
    
    if(rc + rstep <= rmax)
    {
     bottom = varray[(rc+rstep) * numcols + cc];
     ilocal++;
    }
    else
     bottom = 0;

    if(r == (rmax-rstep))
     varray[(r+rstep) * numcols + cc] = 0;
    else
    {
     perturb = 2 * prange * (float)rand()/(float)RAND_MAX - prange;
     varray[(r+rstep) * numcols + cc] = (top + bottom + left + right)/ilocal 
                                       + perturb;
    }
         
    // (rc,c+cstep) coordinate
    ilocal = 0;
    
    left = varray[rc * numcols + cc];
    ilocal++;
    
    if(cc+cstep <= cmax)
    {
     right = varray[rc * numcols + (cc+cstep)];
     ilocal++;
    }
    else
     right = 0;
    
    top = varray[r * numcols + (c+cstep)];
    ilocal++;
    
    bottom = varray[(r+rstep) * numcols + (c+cstep)];
    ilocal++;
    
    
    if(c == (cmax-cstep))
     varray[rc * numcols + (c+cstep)] = 0;
    else
    {
     perturb = 2 * prange * (float)rand()/(float)RAND_MAX - prange;
     varray[rc * numcols + (c+cstep)] = (top + bottom + left + right)/ilocal 
                                       + perturb;
    }

    // (r,cc) coordinate
    ilocal = 0;
    
    left = varray[r * numcols + c];
    ilocal++;
    
    right = varray[r * numcols + (c+cstep)];
    ilocal++;
   
    if(rc - rstep >= 0)
    {
     top = varray[(rc-rstep) * numcols + cc];
     ilocal++;
    }
    else
     top = 0;
    
    bottom = varray[rc * numcols + cc];
    ilocal++;
   
    if(r == 0)
     varray[r * numcols + cc] = 0;
    else
    {
     perturb = 2 * prange * (float)rand()/(float)RAND_MAX - prange;
     varray[r * numcols + cc] = (top + bottom + left + right)/ilocal 
                                + perturb;
    } 
   }
  }
 }
 return 0;
}


//==================================================================
// getColor
//==================================================================
void getColor(float height, float *color)
{
 if(height + 0.1 * rand()/(float)RAND_MAX < 0.6)
 {
  color[0] = 0.64 * (0.5 + height);
  color[1] = 0.40 * (0.5 + height);
  color[2] = 0.18 * (0.5 + height);
 }

 if(height > 0.6)
 {
  color[0] = 0.9 + 0.1 * rand()/(float)RAND_MAX * height;
  color[1] = 0.9 + 0.1 * rand()/(float)RAND_MAX * height;
  color[2] = 0.9 + 0.1 * rand()/(float)RAND_MAX * height;
 }
 if(height + 0.1 * rand()/(float)RAND_MAX < 0.2)
 {
  color[0] = 0.1 * (0.5 + height);
  color[1] = 0.40 * (0.5 + height);
  color[2] = 0.1 * (0.5 + height);
 }
}


//==================================================================
// createSceneViewer
//==================================================================
void createSceneViewer(int x, int y, int width, int height)
{
 GLfloat fogColor[4] = {1.0, 1.0, 1.0, 1.0};

 glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
 glutInitWindowSize( width, height );
 glutInitWindowPosition(x, y);
 
 terraviewGlobal.window_id = glutCreateWindow( "Terrain Viewer" );
 terraviewGlobal.width = width;
 terraviewGlobal.height = height;
 
 reset_eye();
 
 glutReshapeFunc( reshapeCallback );
 glutDisplayFunc( displayCallback );
 glutKeyboardFunc( keyboardCallback );
 glutPassiveMotionFunc( motionCallback );
 glutMouseFunc( mouseCallback );
 glutIdleFunc( idleCallback );
 glutSetCursor(GLUT_CURSOR_CROSSHAIR);

 glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
 glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
 glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
 glEnable(GL_CULL_FACE);
 glFrontFace(GL_CCW);
 glCullFace(GL_BACK);
 glEnable(GL_BLEND);
 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 glEnable(GL_DEPTH_TEST);
 glEnable(GL_DOUBLE);
 glShadeModel(GL_SMOOTH);
 glEnable(GL_LIGHTING);
 glEnable(GL_LIGHT0);
 glEnable(GL_COLOR_MATERIAL);
 glEnable(GL_FOG);
 {
  glFogi(GL_FOG_MODE, GL_EXP2);
  glFogf(GL_FOG_DENSITY, 0.005);
  glFogfv(GL_FOG_COLOR, fogColor);
  glHint(GL_FOG_HINT, GL_DONT_CARE);
  glFogf(GL_FOG_START, 1.0);
  glFogf(GL_FOG_END, 400.0);
 }
}


void updateScene()
{
 vector3d_t viewDir;
 vector3d_t zoom;
 
 if(terraviewGlobal.zoom != 0)
 {
  viewDir = terraviewGlobal.eye_lookAt - terraviewGlobal.eye_pos;
  zoom = terraviewGlobal.zoom * viewDir * 0.1;
  terraviewGlobal.eye_pos = terraviewGlobal.eye_pos + zoom;
  terraviewGlobal.eye_lookAt = terraviewGlobal.eye_lookAt + zoom;
  glutPostWindowRedisplay(terraviewGlobal.window_id);
 }

 if(terraviewGlobal.reset)
 {
  reset_eye();
  terraviewGlobal.reset = false;
 }

 double ca = cos(terraviewGlobal.azimuth);
 double sa = sin(terraviewGlobal.azimuth);
 double ce = cos(terraviewGlobal.elevation);
 double se = sin(terraviewGlobal.elevation);
 double cr = cos(0.0);
 double sr = sin(0.0);

 viewDir.x = -1.0 * sa * ce;
 viewDir.y = se;
 viewDir.z = ca * ce;

 terraviewGlobal.eye_lookAt = terraviewGlobal.eye_pos + (viewDir * 2.0);

 // Get the upVector
 terraviewGlobal.eye_up.x = ca * sr + sa * se * cr;
 terraviewGlobal.eye_up.y = ce * cr;
 terraviewGlobal.eye_up.z = sa * sr - ca * se * cr;

 // Camera position update over. redraw scene
 glutPostWindowRedisplay(terraviewGlobal.window_id);
}


void displayCallback()
{
 GLfloat positionLight[] = {20.0, 20.0, 20.0, 0.0};
 GLfloat ambientLight[] = {0.3, 0.3, 0.3, 1.0};
 GLfloat diffuseLight[] ={1, 1, 1, 1.0};

 static bool sceneInit = false;
 glutSetWindow(terraviewGlobal.window_id);
 
 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
 // set background to a nice blue
 glClearColor(0.627, 0.741, 0.909, 1.0);

 // Set the camera up
 glMatrixMode(GL_MODELVIEW);
 glLoadIdentity();
  
 gluLookAt(terraviewGlobal.eye_pos.x, 
		terraviewGlobal.eye_pos.y, 
		terraviewGlobal.eye_pos.z, 
		terraviewGlobal.eye_lookAt.x,
		terraviewGlobal.eye_lookAt.y,
		terraviewGlobal.eye_lookAt.z, 
		terraviewGlobal.eye_up.x,
		terraviewGlobal.eye_up.y,
		terraviewGlobal.eye_up.z);

 glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
 glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
 glLightfv (GL_LIGHT0, GL_POSITION, positionLight);

 // Draw the scene
 glPushMatrix();
 if(sceneInit)
 {
  glCallList(1);
 }
 else
 {
  drawTerrain();
  sceneInit = true;
 }
 glPopMatrix();
 glutSwapBuffers();
}


void keyboardCallback(unsigned char key, int mouseX, int mouseY)
{
 static bool wireFrame;
 static bool flatShade;
 // 'q' to quit
 // 'm' mouse control active
 // 'x' exit mouse control
 // 'r' to reset to initial view
 // 'w' wireframe view
 switch(key)
 {
  case 'q':
  case 'Q':
   exit(0);
  break;

  case 'm':
  case 'M':
   terraviewGlobal.activeMouse = true;
  break;

  case 'x':
  case 'X':
   terraviewGlobal.activeMouse = false;
  break;

  case 'r':
  case 'R':
   terraviewGlobal.reset = true;
   terraviewGlobal.activeMouse = false;
   glutWarpPointer(terraviewGlobal.width/2, terraviewGlobal.height/2);
  break;
  case 'w':
  case 'W':
   if(wireFrame)
   {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    wireFrame = false;
   }
   else
   {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    wireFrame = true;
   }
  break;
  case 'f':
  case 'F':
   if(flatShade)
   {
    glShadeModel(GL_FLAT);
    flatShade = false;
   }
   else
   {
    glShadeModel(GL_SMOOTH);
    flatShade = true;
   }
  break;
 }
}

void reshapeCallback(int width, int height)
{
 glutSetWindow(terraviewGlobal.window_id);
 terraviewGlobal.height = height;
 terraviewGlobal.width = width;
 glViewport(0, 0, (GLsizei)width, (GLsizei)height);
 glMatrixMode(GL_PROJECTION);
 glLoadIdentity();
 gluPerspective(60, (GLdouble)width/height, 0.1, 500.0);
 glMatrixMode(GL_MODELVIEW);
}


void motionCallback(int mouseX, int mouseY)
{
 if(terraviewGlobal.activeMouse)
 {
  terraviewGlobal.azimuth =
	M_PI * ( 2 * mouseX/(double)terraviewGlobal.width );
  terraviewGlobal.elevation =
	(M_PI/2) * ( 1 - (2 * mouseY/(double)terraviewGlobal.height) );
  updateScene();
 }
}


void mouseCallback(int button, int buttonState, int mouseX, int mouseY)
{
 if(terraviewGlobal.activeMouse)
 {
  if(buttonState == GLUT_UP)
   terraviewGlobal.zoom = 0;

  if(buttonState == GLUT_DOWN)
  {
   switch(button)
   {
    case GLUT_LEFT_BUTTON:
     terraviewGlobal.zoom = 1;
    break;

    case GLUT_RIGHT_BUTTON:
     terraviewGlobal.zoom = -1;
    break;
   }
  }
  updateScene();
 }
}

void idleCallback()
{
 updateScene();
}

void reset_eye()
{
 terraviewGlobal.eye_pos = vector3d_t(0,10,0);
 terraviewGlobal.eye_lookAt = vector3d_t(0,10,-1);
 terraviewGlobal.eye_up = vector3d_t(0,1,0);
 terraviewGlobal.azimuth = M_PI;
 terraviewGlobal.elevation = 0;
 terraviewGlobal.zoom = 0;
}


//==================================================================
// main
//==================================================================
int main(int argc, char *argv[])
{
 int opt;
 terraviewGlobal.roughness = 0.5;
 terraviewGlobal.seed = 566;
 terraviewGlobal.depth = 8;
 while( (opt = getopt(argc, argv, "r:s:d:")) != -1)
 {
  switch(opt)
  {
   case 'r': // set roughness
    terraviewGlobal.roughness = atof(optarg);
    break;
   case 's': // set seed for random generator
    terraviewGlobal.seed = (int)atoi(optarg);
    break;
   case 'd': // set depth of subdivision
    terraviewGlobal.depth = (int)atoi(optarg);
    break;
   default:
    break;
   }
 }
	glutInit(&argc, argv);
	createSceneViewer(0, 0, 640, 480);
	glutMainLoop();
	return 0;
}
