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
 *  Systems in Motion, Teknobyen, Abels Gate 5, 7030 Trondheim, NORWAY.
 *  <URL:http://www.sim.no/>.
 *
\**************************************************************************/

#include <VolumeViz/render/common/Cvr3DRGBATexture.h>

#include <assert.h>
#include <Inventor/C/tidbits.h>
#include <Inventor/SbName.h>

// *************************************************************************

// Don't set value explicitly to SoType::badType(), to avoid a bug in
// Sun CC v4.0. (Bitpattern 0x0000 equals SoType::badType()).
SoType Cvr3DRGBATexture::classTypeId;

SoType Cvr3DRGBATexture::getTypeId(void) const { return Cvr3DRGBATexture::classTypeId; }
SoType Cvr3DRGBATexture::getClassTypeId(void) { return Cvr3DRGBATexture::classTypeId; }

void * Cvr3DRGBATexture::createInstance(void) { return new Cvr3DRGBATexture; }

// *************************************************************************

void
Cvr3DRGBATexture::initClass(void)
{
  assert(Cvr3DRGBATexture::classTypeId == SoType::badType());
  Cvr3DRGBATexture::classTypeId =
    SoType::createType(CvrRGBATexture::getClassTypeId(), "Cvr3DRGBATexture",
                       Cvr3DRGBATexture::createInstance);
}

Cvr3DRGBATexture::Cvr3DRGBATexture(void)
{
  assert(Cvr3DRGBATexture::classTypeId != SoType::badType());
}

Cvr3DRGBATexture::~Cvr3DRGBATexture()
{
}

// *************************************************************************

// Returns pointer to RGBA buffer. Allocates memory for it if
// necessary.
uint32_t *
Cvr3DRGBATexture::getRGBABuffer(void) const
{
  if (this->rgbabuffer == NULL) {
    // Cast away constness.
    Cvr3DRGBATexture * that = (Cvr3DRGBATexture *)this;
    const SbVec3s dims = this->getDimensions();
    that->rgbabuffer = new uint32_t[dims[0] * dims[1] * dims[2]];
  }

  return this->rgbabuffer;
}

// Blank out unused texture parts, to make sure we don't get any
// artifacts due to fp-inaccuracies when rendering.
void
Cvr3DRGBATexture::blankUnused(const SbVec3s & texsize) const
{
  assert(this->rgbabuffer);
  unsigned short x, y, z;

  const SbVec3s dims = this->getDimensions();

  // Bottom 'slab'
  for (z=texsize[2]; z < dims[2]; z++) {
    for (x=0;x<dims[0];++x) {
      for (y=0;y<dims[1];++y) {
        this->rgbabuffer[(z * dims[0] * dims[1]) +
                         (y * dims[0]) + x] = 0x00000000;

      }
    }
  }

  // Front 'slab'
  for (z=0;z<texsize[2];++z) {
    for (x=0;x<texsize[0];++x) {
      for (y=texsize[1];y<dims[1];++y) {
        this->rgbabuffer[(z * dims[0] * dims[1]) +
                         (y * dims[0]) + x] = 0x00000000;
      }
    }
  }

  // Right 'slab'
  for (z=0;z<texsize[2];++z) {
    for (x=texsize[0];x<dims[0];++x) {
      for (y=0;y<dims[1];++y) {
        this->rgbabuffer[(z * dims[0] * dims[1]) +
                         (y * dims[0]) + x] = 0x00000000;
      }
    }
  }

}

// *************************************************************************
