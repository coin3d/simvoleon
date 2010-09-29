/**************************************************************************\
 *
 *  This file is part of the SIM Voleon visualization library.
 *  Copyright (C) 2003-2004 by Systems in Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  ("GPL") version 2 as published by the Free Software Foundation.
 *  See the file LICENSE.GPL at the root directory of this source
 *  distribution for additional information about the GNU GPL.
 *
 *  For using SIM Voleon with software that can not be combined with
 *  the GNU GPL, and for taking advantage of the additional benefits
 *  of our support services, please contact Systems in Motion about
 *  acquiring a SIM Voleon Professional Edition License.
 *
 *  See <URL:http://www.coin3d.org/> for more information.
 *
 *  Systems in Motion, Postboks 1283, Pirsenteret, 7462 Trondheim, NORWAY.
 *  <URL:http://www.sim.no/>.
 *
\**************************************************************************/

#include <VolumeViz/render/common/Cvr3DPaletteTexture.h>

#include <assert.h>
#include <Inventor/SbName.h>
#include <VolumeViz/misc/CvrCLUT.h>

// *************************************************************************

// Don't set value explicitly to SoType::badType(), to avoid a bug in
// Sun CC v4.0. (Bitpattern 0x0000 equals SoType::badType()).
SoType Cvr3DPaletteTexture::classTypeId;

SoType Cvr3DPaletteTexture::getTypeId(void) const { return Cvr3DPaletteTexture::classTypeId; }
SoType Cvr3DPaletteTexture::getClassTypeId(void) { return Cvr3DPaletteTexture::classTypeId; }

void * Cvr3DPaletteTexture::createInstance(void) { return new Cvr3DPaletteTexture; }

// *************************************************************************

void
Cvr3DPaletteTexture::initClass(void)
{
  assert(Cvr3DPaletteTexture::classTypeId == SoType::badType());
  Cvr3DPaletteTexture::classTypeId =
    SoType::createType(CvrPaletteTexture::getClassTypeId(),
                       "Cvr3DPaletteTexture",
                       Cvr3DPaletteTexture::createInstance);
}

Cvr3DPaletteTexture::Cvr3DPaletteTexture(void)
{
  assert(Cvr3DPaletteTexture::classTypeId != SoType::badType());
}

Cvr3DPaletteTexture::~Cvr3DPaletteTexture()
{
}

// *************************************************************************

// Returns pointer to buffer with 8-bit indices. Allocates memory for
// it if necessary.
uint8_t *
Cvr3DPaletteTexture::getIndex8Buffer(void) const
{  
  if (this->indexbuffer == NULL) {
    // Cast away constness.
    Cvr3DPaletteTexture * that = (Cvr3DPaletteTexture *)this;
    const SbVec3s dims = this->getDimensions();
    
    // FIXME: Allocating a larger buffer than needed. Used for
    // spotting a possible bug where the resulting texture becomes
    // larger than the allocated size pulling in garbage for the last
    // slices.  (20100929 handegar)
    that->indexbuffer = new uint8_t[dims[0] * dims[1] * dims[2]*2];
    memset(that->indexbuffer, 128, dims[0] * dims[1] * dims[2]*2);
  }
  return this->indexbuffer;
}

// Blank out unused texture parts, to make sure we don't get any
// artifacts due to fp-inaccuracies when rendering.
void
Cvr3DPaletteTexture::blankUnused(const SbVec3s & texsize) const
{
  assert(this->indexbuffer);
  unsigned short x, y, z;

  const SbVec3s dims = this->getDimensions();

  // Bottom 'slab'
  for (z=texsize[2]; z < dims[2]; z++) {
    for (x=0;x<dims[0];++x) {
      for (y=0;y<dims[1];++y) {
        this->indexbuffer[(z * dims[0] * dims[1]) + (y * dims[0]) + x] = 0x00;
      }
    }
  }

  // Front 'slab'
  for (z=0;z<texsize[2];++z) {
    for (x=0;x<texsize[0];++x) {
      for (y=texsize[1];y<dims[1];++y) {
        this->indexbuffer[(z * dims[0] * dims[1]) + (y * dims[0]) + x] = 0x00;
      }
    }
  }

  // Right 'slab'
  for (z=0;z<texsize[2];++z) {
    for (x=texsize[0];x<dims[0];++x) {
      for (y=0;y<dims[1];++y) {
        this->indexbuffer[(z * dims[0] * dims[1]) + (y * dims[0]) + x] = 0x00;
      }
    }
  }

}

// *************************************************************************
