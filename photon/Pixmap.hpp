//==================================================================
// Pixmap.hpp      Simple class for PPM image handling routines
//
// Author:         Vilas Kumar Chitrakaran (cvilas@ces.clemson.edu)
//                 Feb 2004
//==================================================================


#ifndef _PIXMAP_HPP_INCLUDED
#define _PIXMAP_HPP_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "data_types.hpp"

using namespace std;


//==================================================================
// enum _fileType    Types of PPM files
//==================================================================
typedef enum _fileType
{
 P2, // 8bpp ascii
 P3, // 24bpp ascii
 P5, // 8 bpp binary
 P6  // 24 bpp binary
}fileType_t;


//==================================================================
// class Pixmap    A class to read and write PPM images
//==================================================================
class Pixmap
{
 public:
	
  // ----- constructors and destructor -----
  Pixmap(int width = 0, int height = 0, rgb_t bkColor = rgb_t(0,0,0) );
  Pixmap(const char *imageFile);
  ~Pixmap();

  // ----- file operations ----
  int open(const char *imageFile);
  int write(char *imageFile, fileType_t type = P6);
  
  // ----- image properties ----
  inline int width() const;
  inline int height() const;
  
  // ----- image data operations ----
  inline rgb_t &operator()(int x, int y);
  inline rgb_t operator()(int x, int y) const;

  // ========== END OF INTERFACE ==========
  
 private:
  int d_width;
  int d_height;
  fileType_t d_fileType;
  rgb_t *d_pixel;
};


//==================================================================
//                  ---- inline functions ----
//==================================================================

int Pixmap::width() const
{
 return d_width;
}

int Pixmap::height() const
{
 return d_height;
}


rgb_t Pixmap::operator()(int x, int y) const
{
 if( x > d_width || x < 1  || y > d_height || y < 0)
 {
  cerr << "Pixmap: ERROR index (" << (double)x << ", " << (double)y << ") out of range." 
       << endl;
  return(rgb_t(0,0,0));
 }
 return d_pixel[(y - 1) * d_width + (x - 1)];
}


rgb_t &Pixmap::operator()(int x, int y)
{
 if( x > d_width || x < 1  || y > d_height || y < 0)
 {
  cerr << "Pixmap: ERROR index (" << (double)x << ", " << (double)y << ") out of range." 
       << endl;
  return d_pixel[d_height * d_width - 1];
 }
 return d_pixel[(y - 1) * d_width + (x - 1)];
}



#endif // ifndef _PIXMAP_HPP_INCLUDED
