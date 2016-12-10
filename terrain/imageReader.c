
#include "imageReader.h"

#include <stdio.h>
#include <stdlib.h>


//==============================================================
// Function to read PPM images
//==============================================================

ImageForGl readPPMImage(char *fileName)
{
 FILE *source; /* file pointer to image file */
 char header[80]; /* ppm image header - should not exceed 80 chars in a line */
 char fileType; /* indicates whether type is P2, P3, P5 or P6*/
 int maximumPixelValue = 0;
 unsigned int index;
 int pixelValue;
 ImageForGl image; /* The image structure that will be returned */

 image.imageData = NULL; /* set pointer to NULL - helps in easy memory deallocation */

 /* Open image file */
 if( (source = fopen(fileName, "rb")) == NULL )
 {
  printf("%s %s %s\n", "readPPMImage: File open operation on ",
  	fileName, " failed. \nExiting...");
  exit(1);
 }

 /* Read the header */
 fgets(header, 80, source);
 fileType = header[1];

 while( fgets(header, 80, source) != NULL && header[0] == '#' ); /* ignore comments */
 sscanf( header, "%d %d", &image.numColumns, &image.numRows );
 while( fgets(header, 80, source) != NULL && header[0] == '#' ); /* ignore comments */
 sscanf( header, "%d", &maximumPixelValue );

 if ( maximumPixelValue != 255 )
 {
  printf("%s\n", "readPPMImage: We have a problem!. The maximum pixel value must be 255,\
  	 but is not.\nI am exiting program...");
  exit(1);
 }

 /* Allocate memory for the image data */
 if ( ( image.imageData = (GLubyte *) calloc( image.numRows * image.numColumns * 3,
 	sizeof(GLubyte)  )  ) == NULL )
 {
  printf("%s\n", "readPPMImage: Enough dynamic memory could not be allocated for the image.\
  	 \nExiting program......");
  exit(1);
 }

 /* Read the pixel values into memory */
 if( fileType == '2' ) /* 8bpp ascii */
 {
  for (index = 0; index < image.numRows * image.numColumns; index ++)
  {
   fscanf( source, "%d", &pixelValue );
   image.imageData[index * 3] =
   image.imageData[index * 3 + 1] =
   image.imageData[index * 3 + 2] = (GLubyte)pixelValue;
  }
 }

 if( fileType == '3' ) /* 24bpp ascii */
 {
  for (index = 0; index < image.numRows * image.numColumns * 3; index ++)
  {
   fscanf( source, "%d", &pixelValue );
   image.imageData[index] = (GLubyte)pixelValue;
  }
 }

 if( fileType == '5' ) /* 8bpp binary */
 {
  for (index = 0; index < image.numRows * image.numColumns; index ++)
  {
   fscanf( source, "%c", &pixelValue );
   image.imageData[index * 3] =
   image.imageData[index * 3 + 1] =
   image.imageData[index * 3 + 2] = (GLubyte)pixelValue;
  }
 }


 if( fileType == '6' ) /* 24bpp binary */
 {
  fread( image.imageData, sizeof(GLuint), image.numRows * image.numColumns * 3, source );
 }

 /* Close the file and return */
 fclose(source);
 return image;
}


//==============================================================
// Function to free image memory
//==============================================================
void freeImageMemory( ImageForGl *image)
{
 if (image->imageData != NULL)
  free(image->imageData);
}
