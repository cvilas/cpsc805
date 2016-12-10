//==================================================================
// PhotonMap.hpp  The photon map data structure
//
// Author:        Henrik Wann Jenson - Feb 2001
//                Vilas Kumar Chitrakaran (cvilas@ces.clemson.edu)
//                - April 2004
//==================================================================

#ifndef _PHOTONMAP_HPP_INCLUDED
#define _PHOTONMAP_HPP_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "data_types.hpp"


//==================================================================
// struct _photon 
// This is the photon. The power is not compressed, 
// hence size is 28 bytes
//==================================================================
typedef struct _photon
{
 float pos[3];              // position
 short plane;                 // splitting plane for kd-tree
 unsigned char theta, phi;    // incoming direction
 float power[3];            // photon power (uncompressed)
}photon_t;



//==================================================================
// struct _nearest_photons  
// This structure is used only to locate the nearest photons
//==================================================================
typedef struct _nearest_photons
{
 int max;
 int found;
 int got_heap;
 float pos[3];
 float *dist2;
 const photon_t **index;
}nearest_photons_t;


//==================================================================
// class PhotonMap  The photon map class
//==================================================================
class PhotonMap
{
 public:
  PhotonMap(const int maxPhot = 1);

  ~PhotonMap();

  int init(const int maxPhot);
   // Initialize the photon map with max number of 
   // photons to be used.
   //  return 0 if success, else -1;
   
  void store(const rgb_t power, const vector3d_t pos, 
             const vector3d_t dir);
   // Puts a photon into the flat array that will form the final kd-tree
   // Call this function to store a photon
   
  void scalePhotonPower(const float scale); 
   // Scale the power of all photons once they have been 
   // emitted from the light source. scale = 1/(number of emitted photons)
   // Call this function after each light source is processed
   
  void balance(void); 
   // Creates a left-handed kd-tree from the flat photon array
   // Call this function before using the photon map for rendering
   
  rgb_t irradianceEstimate(const vector3d_t pos, 
                          const vector3d_t normal, const float maxDist, 
                          const int nPhotons) const;
   // Computes an irradiance estimate at a given surface position
   // return: irradiance
   // pos: surface position
   // normal: surface normal at pos
   // maxDist: max distance to look for photons
   // nPhotons: number of photons to use
  
  void locatePhotons(nearest_photons_t *const np, const int index) const;
   // Finds the nearest photons in the photon 
   // map given np
   // call with index = 1  
   
  vector3d_t photonDir(const photon_t *p) const;
   // return: direction of photon
   // p: the photon

 private:
  void balanceSegment(photon_t **pbal, photon_t **porg, const int index,
                      const int start, const int end);
   // See book "Realistic Image Synthesis using 
   // Photon Mapping" chapter 6 for explanation of this function
  
  void medianSplit(photon_t **p, const int start, const int end, 
                   const int median, const int axis);
   // Splits the photon array into two separate pieces around 
   // the median, with all photons below the median in the lower half 
   // and all photons above the median in the upper half. The 
   // comparison criteria is the axis (indicated by the axis parameter)
   // (inspired by routine in "Algorithms in C++" by Sedgewick)
  
  photon_t *d_photons;
  int d_storedPhotons;
  int d_halfStoredPhotons;
  int d_maxPhotons;
  int d_prevScale;
  float d_cosTheta[256];
  float d_sinTheta[256];
  float d_cosPhi[256];
  float d_sinPhi[256];
  float d_bboxMin[3];
  float d_bboxMax[3];
};

#endif // _PHOTONMAP_HPP_INCLUDED

