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

#include <Inventor/actions/SoGLRenderAction.h>
#include <VolumeViz/elements/CvrVoxelBlockElement.h>

#include "CvrRaycastTexture.h"

#include <RenderManager.h>
#include <VoxelDataID.h>

CvrRaycastTexture::CvrRaycastTexture()
  : refcount(0)
{
}


CvrRaycastTexture::~CvrRaycastTexture()
{
}


const CvrRaycastTexture *
CvrRaycastTexture::create(CLVol::RenderManager * rm,
                          const SoGLRenderAction * action,
                          const SbBox3s cut)
{
  CvrRaycastTexture * tex = new CvrRaycastTexture();
  tex->bbox = cut;

  const CvrVoxelBlockElement * vbelem = CvrVoxelBlockElement::getInstance(action->getState());
  unsigned int bytespervoxel = vbelem->getBytesPrVoxel();  
  unsigned char * rawdata = CvrRaycastTexture::buildCube(vbelem->getVoxelCubeDimensions(), cut, 
                                                         bytespervoxel,
                                                         vbelem->getVoxels());

  SbVec3s size = cut.getSize();  
  tex->voxeldataid = rm->createVoxelData(size[0], size[1], size[2], 8*bytespervoxel, 
                                         (const unsigned char *) rawdata);
  delete rawdata; // Data has been transferred to libCLVol. 
  return tex;
}


unsigned char * 
CvrRaycastTexture::buildCube(SbVec3s dim, SbBox3s cutcube, unsigned int bytespervoxel, const uint8_t * data)
{
  SbVec3s ccmin, ccmax;
  cutcube.getBounds(ccmin, ccmax);

  const int nrhorizvoxels = ccmax[0] - ccmin[0];
  const int nrvertvoxels = ccmax[1] - ccmin[1];
  const int nrdepthvoxels = ccmax[2] - ccmin[2];

  assert(nrhorizvoxels > 0);
  assert(nrvertvoxels > 0);
  assert(nrdepthvoxels > 0);

  const SbVec3s outputdims(nrhorizvoxels, nrvertvoxels, nrdepthvoxels);
  const unsigned int staticoffset = (ccmin[2] * dim[0] * dim[1]) + (ccmin[1] * dim[0]) + ccmin[0];

  const unsigned int voxelsize = sizeof(uint8_t)*bytespervoxel;
  uint8_t * output = new uint8_t[nrhorizvoxels* nrvertvoxels*nrdepthvoxels*bytespervoxel];

  for (int depthidx = 0; depthidx < nrdepthvoxels; depthidx++) {
    for (int rowidx = 0; rowidx < nrvertvoxels; rowidx++) {
      const unsigned int inoffset = staticoffset + (rowidx * dim[0]) + (depthidx * dim[0]*dim[1]);
      const uint8_t * srcptr = &(data[inoffset * voxelsize]);
      uint8_t * dstptr = &(output[((depthidx * nrhorizvoxels * nrvertvoxels) + (nrhorizvoxels * rowidx)) * voxelsize]);
      (void) memcpy(dstptr, srcptr, nrhorizvoxels * voxelsize);
    }
  }

  return output;
}


uint32_t
CvrRaycastTexture::getRefCount() const
{
  return this->refcount;
}


void
CvrRaycastTexture::ref() const
{
  ((CvrRaycastTexture *)this)->refcount++;
}


void
CvrRaycastTexture::unref() const
{
  ((CvrRaycastTexture *)this)->refcount--;
  if (this->refcount == 0) { delete this; }
}


const SbVec3s &
CvrRaycastTexture::getDimensions() const
{
  return this->bbox.getSize();
}


const CLVol::VoxelDataID * 
CvrRaycastTexture::getVoxelDataId() const
{
  return this->voxeldataid;
}
