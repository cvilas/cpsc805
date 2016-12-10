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

using namespace std;

//==================================================================
// struct _rgb  A simple structure to represent colors.
//==================================================================
typedef struct _rgb
{
 _rgb(double R = 0, double G = 0, double B = 0) : r(R), g(G), b(B) {};
 double r;
 double g;
 double b;
}rgb_t;


//==================================================================
// Operators for _rgb
//==================================================================
rgb_t operator+(const rgb_t &c1, const rgb_t &c2);
rgb_t operator*(const double &scalar, const rgb_t &c);
rgb_t operator*(const rgb_t &c, const double &scalar);


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
  inline rgb_t &operator()(int x, int y) const;

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


rgb_t &Pixmap::operator()(int x, int y) const
{
 if( x > d_width || x < 1  || y > d_height || y < 1)
 {
  //cerr << "Pixmap: ERROR index (" << (double)x << ", " 
  //     << (double)y << ") out of range." 
  //     << endl;
  return d_pixel[d_height * d_width];
 }
 return d_pixel[(y - 1) * d_width + (x - 1)];
}



#endif // ifndef _PIXMAP_HPP_INCLUDED
