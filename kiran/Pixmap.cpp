//==================================================================
// Pixmap.cpp      
//
// Author:         Vilas Kumar Chitrakaran (cvilas@ces.clemson.edu)
//                 Feb 2004
//==================================================================

#include "Pixmap.hpp"
#include <math.h>

//==================================================================
// operator+ for rgb_t
//==================================================================
rgb_t operator+(const rgb_t &v1, const rgb_t &v2)
{
 rgb_t color;
 color.r = v1.r + v2.r;
 color.g = v1.g + v2.g;
 color.b = v1.b + v2.b;
 if(color.r > 1.0) color.r = 1.0;
 if(color.g > 1.0) color.g = 1.0;
 if(color.b > 1.0) color.b = 1.0;
 return color;
}


//==================================================================
// operator* for rgb_t
//==================================================================
rgb_t operator*(const double &scalar, const rgb_t &c)
{
 rgb_t color;
 double factor = fabs(scalar);
 color.r = c.r * factor;
 color.g = c.g * factor;
 color.b = c.b * factor;
 if(color.r > 1.0) color.r = 1.0;
 if(color.g > 1.0) color.g = 1.0;
 if(color.b > 1.0) color.b = 1.0;
 return color; 
}


rgb_t operator*(const rgb_t &c, const double &scalar)
{
 rgb_t color;
 double factor = fabs(scalar);
 color.r = c.r * factor;
 color.g = c.g * factor;
 color.b = c.b * factor;
 if(color.r > 1.0) color.r = 1.0;
 if(color.g > 1.0) color.g = 1.0;
 if(color.b > 1.0) color.b = 1.0;
 return color; 
}


//==================================================================
// Pixmap::Pixmap
//==================================================================
Pixmap::Pixmap(int width, int height, rgb_t bkColor)
{
 d_pixel = NULL;
 d_width = width;
 d_height = height;

 if(d_width <= 0 || d_height <= 0)
  return;
		
 d_pixel = (rgb_t *)calloc( d_width * d_height + 1, sizeof(rgb_t) );
 if(d_pixel == NULL)
 {
  cerr << "Pixmap: ERROR allocating memory." << endl;
  exit(-1);
 }
	
 for (int i = 0; i < d_width * d_height; i++)
 {
  d_pixel[i] = bkColor;
 }
}


Pixmap::Pixmap(const char *imageFile)
{
 d_pixel = NULL;
	
 // Open image file
 if( open(imageFile) != 0 )
  exit(-1);
}


//==================================================================
// Pixmap::~Pixmap
//==================================================================
Pixmap::~Pixmap()
{
 //free allocated memory
 if ( d_pixel != NULL)
  free(d_pixel);
}


//==================================================================
// Pixmap::open
//==================================================================
int Pixmap::open(const char *fileName)
{
 FILE *source;
 char header[80];
 int maxPixVal = 0;
 int pixVal;
 unsigned char cPixVal;
 char fileType;
 
 d_pixel = NULL;
	
 // Open image file
 if( (source = fopen(fileName, "rb")) == NULL )
 {
  cerr << "Pixmap: ERROR opening image file." << endl;
  return(-1);
 }
	
 // Read the header
 fgets(header, 80, source);
 fileType = header[1]; // indicates whether type is P2, P3, P5 or P6
	
 while( fgets(header, 80, source) != NULL && header[0] == '#' ); // ignore comments
 sscanf( header, "%d %d", &d_width, &d_height );
 while( fgets(header, 80, source) != NULL && header[0] == '#' ); // ignore comments
 sscanf( header, "%d", &maxPixVal );
 
 if ( maxPixVal != 255 )
 {
  cerr << "Pixmap: ERROR reading image header." << endl;
  exit(1);
 }
 
 // Allocate memory for the image data
 d_pixel = (rgb_t *) realloc( d_pixel, (1 + d_width * d_height) * sizeof(rgb_t) );
 if(d_pixel == NULL)
 {
  cout << "Pixmap: ERROR allocating memory." << endl;
  exit(-1);
 }
 
 // Read the pixel values into memory
 for (int i = 0; i < d_width * d_height; i++)
 {
  if( d_fileType == '2' ) // 8bpp ascii
  {
   fscanf( source, "%d", &pixVal );
   d_pixel[i].r =
   d_pixel[i].g = 
   d_pixel[i].b = pixVal/255.0;
  }
  
  if( fileType == '3' ) // 24bpp ascii
  {
   fscanf( source, "%d", &pixVal );
   d_pixel[i].r = pixVal/255.0;
   fscanf( source, "%d", &pixVal );
   d_pixel[i].g = pixVal/255.0;
   fscanf( source, "%d", &pixVal );
   d_pixel[i].b = pixVal/255.0;
  }
	
  if( fileType == '5' ) // 8bpp binary
  {
   fscanf( source, "%c", &cPixVal );
   d_pixel[i].r = 
   d_pixel[i].g =
   d_pixel[i].b = cPixVal/255.0;
  }
  
  if( fileType == '6' ) // 24bpp binary
  {
   fscanf( source, "%c", &cPixVal );
   d_pixel[i].r = cPixVal/255.0;
   fscanf( source, "%c", &cPixVal );
   d_pixel[i].g = cPixVal/255.0;
   fscanf( source, "%c", &cPixVal );
   d_pixel[i].b = cPixVal/255.0;
  }
 }
 fclose(source);
 return 0;
}


//==================================================================
// Pixmap::write
//==================================================================
int Pixmap::write(char *fileName, fileType_t type)
{
 FILE *destination;
 char *header = 0;
 int val, r, g, b;
	
 destination = fopen(fileName, "w+");

 if ( destination == NULL)
 {
  cerr << "Pixmap: ERROR opening image file." << endl;
  return -1;
 }
	
 // write header
 if(type == P2) // 8bpp ascii
  header = "P2";
 if(type == P5) // 8bpp binary
  header = "P5";
 if(type == P3) // 24bpp ascii
  header = "P3";
 if(type == P6) // 24bpp binary
  header = "P6";
		
 fprintf(destination, "%s %d %d %d\n", header, d_width, d_height, 255);

 for (int i = 0; i < d_width * d_height; i++)
 {
  if(type == P2) // 8bpp ascii
  {
   val = (int)((d_pixel[i].r + d_pixel[i].g + d_pixel[i].b) * 85);
   if(val > 0xFF) val = 0xFF;
   fprintf(destination, "%d ", val);
   if(i%10 == 0)
    fprintf(destination, "\n");
   }
   
   if(type == P5) // 8bpp binary
   {
    val = (int)((d_pixel[i].r + d_pixel[i].g + d_pixel[i].b) * 85);
    if(val > 0xFF) val = 0xFF;
    fprintf(destination, "%c", (unsigned char)val);
   }
   
   if(type == P3) // 24bpp ascii
   {
    r = (int)(d_pixel[i].r * 255); if (r > 0xFF) r = 0xFF;
    g = (int)(d_pixel[i].g * 255); if (g > 0xFF) g = 0xFF;
    b = (int)(d_pixel[i].b * 255); if (b > 0xFF) b = 0xFF;
    fprintf( destination, "%d %d %d ", r, g, b);
    if(i%10 == 0)
     fprintf(destination, "\n");
   }
   
   if(type == P6) // 24bpp binary
   {
    r = (int)(d_pixel[i].r * 255); if (r > 0xFF) r = 0xFF;
    g = (int)(d_pixel[i].g * 255); if (g > 0xFF) g = 0xFF;
    b = (int)(d_pixel[i].b * 255); if (b > 0xFF) b = 0xFF;
    fprintf( destination, "%c%c%c", (unsigned char)r, 
                                    (unsigned char)g, 
                                    (unsigned char)b);
  }
 }
 fclose(destination);
 return 0;
}

