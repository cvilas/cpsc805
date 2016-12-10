#ifndef IMAGEREADER_H
#define IMAGEREADER_H

#include <GL/gl.h>

/*---------------------------------------------------------------
 ImageForGl - A structure for image data
  - A simple structure for storing pixel values read from
    image files, using OpenGL data types.
---------------------------------------------------------------*/

typedef struct
{
 GLuint numRows;
 GLuint numColumns;
 GLubyte *imageData;
}ImageForGl;


/*==============================================================
 Functions to read images
==============================================================*/

ImageForGl readPPMImage(char *fileName);
 // This function reads PPM images encoded as P2 (8bpp, ASCII),
 // P3 (24bpp, ASCII), P5 (8bpp, binary) or P6 (24bpp, binary).
 //  return - imageForGl structure containing image data


void freeImageMemory( ImageForGl *image);
 // This function frees up memory allocated to image data.
 // Make sure you call this function at the end of your program
 // to de-allocate memory used to store pixel data.
 
// ========== END OF INTERFACE ==========

#endif /* IMAGEREADER_H */

