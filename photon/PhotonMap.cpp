#include "PhotonMap.hpp"
#include <iostream>

//==================================================================
PhotonMap::PhotonMap(const int maxPhot)
//==================================================================
{
 d_photons = NULL;
 
 if(init(maxPhot) == -1)
  exit(-1);
 
 d_bboxMin[0] = d_bboxMin[1] = d_bboxMin[2] = 1e8f;
 d_bboxMax[0] = d_bboxMax[1] = d_bboxMax[2] = -1e8f;
 
 // initialize direction conversion tables
 for(int i = 0; i < 256; i++)
 {
  double angle = double (i) * (1.0/256.0) * M_PI;
  d_cosTheta[i] = cos(angle);
  d_sinTheta[i] = sin(angle);
  d_cosPhi[i] = cos(2.0 * angle);
  d_sinPhi[i] = sin(2.0 * angle); 
 }
}


//==================================================================
int PhotonMap::init(const int maxPhot)
//==================================================================
{
 d_storedPhotons = 0;
 d_prevScale = 1;
 d_maxPhotons = maxPhot;
 
 if(d_photons)
 {
  free(d_photons);
  d_photons = NULL;
 }
  
 d_photons = (photon_t *)malloc( (d_maxPhotons + 1) * sizeof(photon_t) );
 if(d_photons == NULL)
 {
  cerr << "PhotonMap: ERROR initializing memory for photon map." 
       << endl;
  return(-1);
 }
 return 0;
}


//==================================================================
PhotonMap::~PhotonMap()
//==================================================================
{
 free(d_photons);
}


//==================================================================
void PhotonMap::store(const rgb_t powe, const vector3d_t posi, 
                      const vector3d_t direc)
//==================================================================
{
 float power[3], pos[3], dir[3];
 power[0] = powe.r; power[1] = powe.g; power[2] = powe.b;
 pos[0] = posi.x; pos[1] = posi.y; pos[2] = posi.z;
 dir[0] = direc.x; dir[1] = direc.y; dir[2] = direc.z;
 
 if(d_storedPhotons > d_maxPhotons)
  return;
 
 d_storedPhotons++;
 
 photon_t *const node = &d_photons[d_storedPhotons];
 for (int i=0; i<3; i++) 
 {
  node->pos[i] = pos[i];
  if (node->pos[i] < d_bboxMin[i])
   d_bboxMin[i] = node->pos[i];
  if (node->pos[i] > d_bboxMax[i])
   d_bboxMax[i] = node->pos[i];
  node->power[i] = power[i];
 }
  
 int theta = int( acos(dir[2])*(256.0/M_PI) );
 if (theta>255)
  node->theta = 255;
 else
  node->theta = (unsigned char)theta;
  
 int phi = int( atan2(dir[1],dir[0])*(256.0/(2.0*M_PI)) );
 if (phi>255)
  node->phi = 255;
 else if (phi<0)
  node->phi = (unsigned char)(phi+256);
 else
  node->phi = (unsigned char)phi;
}



//==================================================================
void PhotonMap::scalePhotonPower(const float scale)
//==================================================================
{
 for (int i=d_prevScale; i<=d_storedPhotons; i++) 
 {
  d_photons[i].power[0] *= scale;
  d_photons[i].power[1] *= scale;
  d_photons[i].power[2] *= scale;
 }
 d_prevScale = d_storedPhotons;
}


//==================================================================
void PhotonMap::balance(void)
//==================================================================
{
  if (d_storedPhotons > 1) {
    // allocate two temporary arrays for the balancing procedure
    photon_t **pa1 = (photon_t**)malloc(sizeof(photon_t*)*(d_storedPhotons+1));
    photon_t **pa2 = (photon_t**)malloc(sizeof(photon_t*)*(d_storedPhotons+1));

    for (int i=0; i<= d_storedPhotons; i++)
      pa2[i] = &d_photons[i];

    balanceSegment( pa1, pa2, 1, 1, d_storedPhotons );
    free(pa2);

    // reorganize balanced kd-tree (make a heap)
    int d, j=1, foo=1;
    photon_t foo_photon = d_photons[j];

    for (int i=1; i<= d_storedPhotons; i++) {
      d=pa1[j]-d_photons;
      pa1[j] = NULL;
      if (d != foo)
        d_photons[j] = d_photons[d];
      else {
        d_photons[j] = foo_photon;

        if (i<d_storedPhotons) {
          for (;foo<=d_storedPhotons; foo++)
            if (pa1[foo] != NULL)
              break;
          foo_photon = d_photons[foo];
          j = foo;
        }
        continue;
      }
      j = d;
    }
    free(pa1);
  }

  d_halfStoredPhotons = d_storedPhotons/2-1;
}


//==================================================================
rgb_t PhotonMap::irradianceEstimate(const vector3d_t pos, 
                          const vector3d_t normal, const float maxDist, 
                          const int nPhotons) const
//==================================================================
{
 rgb_t irrad;
 
 nearest_photons_t np;
 np.dist2 = (float *)alloca( sizeof(float) * (nPhotons + 1) );
 np.index = (const photon_t **)alloca( sizeof(photon_t *) * (nPhotons + 1) );
 
 np.pos[0] = pos.x; np.pos[1] = pos.y; np.pos[2] = pos.z;
 np.max = nPhotons;
 np.found = 0;
 np.got_heap = 0;
 np.dist2[0] = maxDist * maxDist;
 
 // locate the nearest d_photons
 locatePhotons( &np, 1 );
 
 // if less than 8 d_photons return
 if( np.found < 8 )
  return irrad;
 
 vector3d_t pDir;
 
 // sum irradiance from all d_photons
 for(int i = 1; i <= np.found; i++)
 {
  const photon_t *p = np.index[i];
  
  // the photonDir() call and the following if can be 
  // omitted for speed if the scene does not have any
  // thin surfaces
  pDir = photonDir(p);
  if( dot(pDir, normal) < 0.0f )
  {
   irrad.r += p->power[0];
   irrad.g += p->power[1];
   irrad.b += p->power[2];
  }
 }
 
 const float tmp = (1.0f/M_PI)/(np.dist2[0]); // estimate of density
 
 irrad = irrad * tmp;
 return irrad;
}


//==================================================================
void PhotonMap::locatePhotons(nearest_photons_t *const np, 
                              const int index) const
//==================================================================
{
  const photon_t *p = &d_photons[index];
  float dist1;

  if (index < d_halfStoredPhotons) {
    dist1 = np->pos[ p->plane ] - p->pos[ p->plane ];

    if (dist1>0.0) { // if dist1 is positive search right plane
      locatePhotons( np, 2*index+1 );
      if ( dist1*dist1 < np->dist2[0] )
        locatePhotons( np, 2*index );
    } else {         // dist1 is negative search left first
      locatePhotons( np, 2*index );
      if ( dist1*dist1 < np->dist2[0] )
        locatePhotons( np, 2*index+1 );
    }
  }

  // compute squared distance between current photon and np->pos

  dist1 = p->pos[0] - np->pos[0];
  float dist2 = dist1*dist1;
  dist1 = p->pos[1] - np->pos[1];
  dist2 += dist1*dist1;
  dist1 = p->pos[2] - np->pos[2];
  dist2 += dist1*dist1;
  
  if ( dist2 < np->dist2[0] ) {
    // we found a photon  [:)] Insert it in the candidate list

    if ( np->found < np->max ) {
      // heap is not full; use array
      np->found++;
      np->dist2[np->found] = dist2;
      np->index[np->found] = p;
    } else {
      int j,parent;

      if (np->got_heap==0) { // Do we need to build the heap?
        // Build heap
        float dst2;
        const photon_t *phot;
        int half_found = np->found>>1;
        for ( int k=half_found; k>=1; k--) {
          parent=k;
          phot = np->index[k];
          dst2 = np->dist2[k];
          while ( parent <= half_found ) {
            j = parent+parent;
            if (j<np->found && np->dist2[j]<np->dist2[j+1])
              j++;
            if (dst2>=np->dist2[j])
              break;
            np->dist2[parent] = np->dist2[j];
            np->index[parent] = np->index[j];
            parent=j;
          }
          np->dist2[parent] = dst2;
          np->index[parent] = phot;
        }
        np->got_heap = 1;
      }

      // insert new photon into max heap
      // delete largest element, insert new and reorder the heap

      parent=1;
      j = 2;
      while ( j <= np->found ) {
        if ( j < np->found && np->dist2[j] < np->dist2[j+1] )
          j++;
        if ( dist2 > np->dist2[j] )
          break;
        np->dist2[parent] = np->dist2[j];
        np->index[parent] = np->index[j];
        parent = j;
        j += j;
      }
      np->index[parent] = p;
      np->dist2[parent] = dist2;

      np->dist2[0] = np->dist2[1];
    }
  }
}


//==================================================================
vector3d_t PhotonMap::photonDir(const photon_t *p) const
//==================================================================
{
 vector3d_t dir;
 dir.x = d_sinTheta[p->theta] * d_cosPhi[p->phi];
 dir.y = d_sinTheta[p->theta] * d_sinPhi[p->phi];
 dir.z = d_cosTheta[p->theta];
 return dir;
}


//==================================================================
void PhotonMap::balanceSegment(photon_t **pbal, photon_t **porg, 
                               const int index, const int start, 
                               const int end)
//==================================================================
{
  //--------------------
  // compute new median
  //--------------------

  int median=1;
  while ((4*median) <= (end-start+1))
    median += median;

  if ((3*median) <= (end-start+1)) {
    median += median;
    median += start-1;
  } else	
    median = end-median+1;

  //--------------------------
  // find axis to split along
  //--------------------------

  int axis=2;
  if ((d_bboxMax[0]-d_bboxMin[0])>(d_bboxMax[1]-d_bboxMin[1]) &&
(d_bboxMax[0]-d_bboxMin[0])>(d_bboxMax[2]-d_bboxMin[2]))
    axis=0;
  else if ((d_bboxMax[1]-d_bboxMin[1])>(d_bboxMax[2]-d_bboxMin[2]))
    axis=1;

  //------------------------------------------
  // partition photon block around the median
  //------------------------------------------

  medianSplit( porg, start, end, median, axis );

  pbal[ index ] = porg[ median ];
  pbal[ index ]->plane = axis;

  //----------------------------------------------
  // recursively balance the left and right block
  //----------------------------------------------

  if ( median > start ) {
    // balance left segment
    if ( start < median-1 ) {
      const float tmp=d_bboxMax[axis];
      d_bboxMax[axis] = pbal[index]->pos[axis];
      balanceSegment( pbal, porg, 2*index, start, median-1 );
      d_bboxMax[axis] = tmp;
    } else {
      pbal[ 2*index ] = porg[start];
    }
  }

  if ( median < end ) {
    // balance right segment
    if ( median+1 < end ) {
      const float tmp = d_bboxMin[axis];		
      d_bboxMin[axis] = pbal[index]->pos[axis];
      balanceSegment( pbal, porg, 2*index+1, median+1, end );
      d_bboxMin[axis] = tmp;
    } else {
      pbal[ 2*index+1 ] = porg[end];
    }
  }	
}


void swap(photon_t **ph, int a, int b) 
{ 
 photon_t *ph2 = ph[a]; ph[a] = ph[b]; ph[b] = ph2; 
} 

//==================================================================
void PhotonMap::medianSplit(photon_t **p, const int start, 
                            const int end, const int median, 
                            const int axis)
//==================================================================
{
  int left = start;
  int right = end;

  while ( right > left ) {
    const float v = p[right]->pos[axis];
    int i=left-1;
    int j=right;
    for (;;) {
      while ( p[++i]->pos[axis] < v )
        ;
      while ( p[--j]->pos[axis] > v && j>left )
        ;
      if ( i >= j )
        break;
      swap(p,i,j);
    }

    swap(p,i,right);
    if ( i >= median )
      right=i-1;
    if ( i <= median )
      left=i+1;
  }
}

