/**************************************************************************\
 *
 *  This file is part of the SIM Voleon visualization library.
 *  Copyright (C) by Kongsberg Oil & Gas Technologies.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  ("GPL") version 2 as published by the Free Software Foundation.
 *  See the file LICENSE.GPL at the root directory of this source
 *  distribution for additional information about the GNU GPL.
 *
 *  For using SIM Voleon with software that can not be combined with
 *  the GNU GPL, and for taking advantage of the additional benefits
 *  of our support services, please contact Kongsberg Oil & Gas
 *  Technologies about acquiring a SIM Voleon Professional Edition
 *  License.
 *
 *  See http://www.coin3d.org/ for more information.
 *
 *  Kongsberg Oil & Gas Technologies, Bygdoy Alle 5, 0257 Oslo, NORWAY.
 *  http://www.sim.no/  sales@sim.no  coin-support@coin3d.org
 *
\**************************************************************************/

/*!
  This class is used for setting up and handling subcube-textures
  which will be used for rendering by the libCLVol library.

  NOTE: A lot of this code is copied from and inspired by the
  Cvr3DTexCube class, but somewhat refined. The code in Cvr3DTexCube
  will, in the long run, become obsolete when SIMVoleon somewhere in
  the furure relies on raycasting for volemendering of gigabyte-sized
  datasets. This is why we don't just build on the old structure, but
  start fresh instead.

 */

#include "CvrRaycastCube.h"

#include <Inventor/actions/SoGLRenderAction.h>

#include <VolumeViz/elements/CvrDataSizeElement.h>
#include <VolumeViz/elements/CvrVoxelBlockElement.h>
#include <VolumeViz/render/raycast/CvrRaycastSubCube.h>
#include <VolumeViz/misc/CvrUtil.h>


class SubCube {
public:
  SubCube(CvrRaycastSubCube * p) { this->cube = p; }
  CvrRaycastSubCube * cube;
  double distancefromcamera;
  double cameraplane2cubecenter;
  float boundingsphereradius;    
};


CvrRaycastCube::CvrRaycastCube(const SoGLRenderAction * action)
  : subcubes(NULL)
{
  SoState * state = action->getState();
  this->subcubesize =
    CvrUtil::clampSubCubeSize(CvrDataSizeElement::get(state));
  
  if (CvrUtil::doDebugging()) {
    SoDebugError::postInfo("CvrRaycastCube::CvrRaycastCube",
                           "subcubedimensions==<%d, %d, %d>",
                           this->subcubesize[0],
                           this->subcubesize[1],
                           this->subcubesize[2]);
  }
  
  const CvrVoxelBlockElement * vbelem = CvrVoxelBlockElement::getInstance(state);
  const SbVec3s & dim = vbelem->getVoxelCubeDimensions();
  assert(dim[0] > 0 && dim[1] > 0 && dim[2] > 0 && "Invalid voxel block size");

  this->dimensions = dim;
  this->origo = SbVec3f(-((float) dim[0]) / 2.0f, 
                        -((float) dim[1]) / 2.0f, 
                        -((float) dim[2]) / 2.0f);

  //Note: cols and rows were swiched in Cvr3DTexCube
  this->nrsubcubes[0] = (this->dimensions[0] + this->subcubesize[0] - 1) / this->subcubesize[0];
  this->nrsubcubes[1] = (this->dimensions[1] + this->subcubesize[1] - 1) / this->subcubesize[1];
  this->nrsubcubes[2] = (this->dimensions[2] + this->subcubesize[2] - 1) / this->subcubesize[2];

}


CvrRaycastCube::~CvrRaycastCube()
{
  this->releaseAllSubCubes();
}


void 
CvrRaycastCube::render(const SoGLRenderAction * action)
{
  /*
    TODO:
    * Fetch data from element.
    * Split into subcubes according to size-limitations
      * Cache subcubes for later reuse
    * Sort Subcubes according to distance from camera
    * Setup CLVol
    * Clear CLVol's framebuffer
      * Render all subcubes
    * draw CLVol framebuffer result

    // FIXME: What about intersections with existing OpenGL tris? (20100824 handegar)
   */
}


SubCube * 
CvrRaycastCube::getSubCube(SoState * state, 
                           unsigned int row, 
                           unsigned int col, 
                           unsigned int depth)
{
  assert(row < this->nrsubcubes[0]);
  assert(col < this->nrsubcubes[1]);
  assert(depth < this->nrsubcubes[2]);
  if (this->subcubes == NULL) return NULL;

  const unsigned int idx = this->getSubCubeIdx(row, col, depth);
  SubCube * subp = this->subcubes[idx];

  if (subp) {
    // **
    // FIXME: Check if this subcube is still valid/relevant. If not,
    // delete/release it and return NULL.
    // **
  }
  else {
    // No subcube created yet for this row, col, depth.
  }

  return subp;
}


SubCube * 
CvrRaycastCube::buildSubCube(const SoGLRenderAction * action,
                             const SbVec3f & origo,
                             unsigned int row,
                             unsigned int col,
                             unsigned int depth)
{
  assert((this->getSubCube(action->getState(), col, row, depth) == NULL) && 
         "Subcube already created!");

  // No subcubes created yet?
  if (this->subcubes == NULL) {
    if (CvrUtil::doDebugging()) {
      SoDebugError::postInfo("CvrRaycastCube::buildSubCube",
                             "number of subcubes needed == %d (%d x %d x %d)",
                             this->nrsubcubes[0]*this->nrsubcubes[1]*this->nrsubcubes[2],
                             this->nrsubcubes[0], this->nrsubcubes[1], this->nrsubcubes[2]);
    }
    
    this->subcubes = new SubCube*[this->nrsubcubes[0]*this->nrsubcubes[1]*this->nrsubcubes[2]];
    for (unsigned int i=0; i < this->nrsubcubes[0]; i++) {
      for (unsigned int j=0; j < this->nrsubcubes[1]; j++) {
        for (unsigned int k=0; k < this->nrsubcubes[2]; k++) {
          const unsigned int idx = this->getSubCubeIdx(i, j, k);
          this->subcubes[idx] = NULL; // Reset to NULL
        }
      }
    }
  }
        
  // **
  // FIXME: Create new CvrRaycastSubCube instance. Fill with data. (20100824 handegar)
  // **

  return NULL;
}


unsigned int
CvrRaycastCube::getSubCubeIdx(unsigned int row, 
                              unsigned int col, 
                              unsigned int depth) const
{
  assert(row < this->nrsubcubes[0]);
  assert(col < this->nrsubcubes[1]);
  assert(depth < this->nrsubcubes[2]);

  unsigned int idx =
    (col + (row * this->nrsubcubes[1])) + (depth * this->nrsubcubes[0] * this->nrsubcubes[1]);
  
  assert(idx < (this->nrsubcubes[0] * this->nrsubcubes[1] * this->nrsubcubes[2]));
  return idx;
}


void 
CvrRaycastCube::releaseSubCube(unsigned int row, 
                               unsigned int column, 
                               unsigned int depth)
{
  const int idx = this->getSubCubeIdx(row, column, depth);
  SubCube * p = this->subcubes[idx];
  if (p) {
    this->subcubes[idx] = NULL;
    delete p->cube;
    delete p;
  }
}


void
CvrRaycastCube::releaseAllSubCubes()
{
  if (this->subcubes == NULL) return;
  
  for (unsigned int row = 0; row < this->nrsubcubes[0]; row++) {
    for (unsigned int col = 0; col < this->nrsubcubes[1]; col++) {
      for (unsigned int depth = 0; depth < this->nrsubcubes[2]; depth++) {
        this->releaseSubCube(row, col, depth);
      }
    }
  }
  
  delete[] this->subcubes;
  this->subcubes = NULL;
}
