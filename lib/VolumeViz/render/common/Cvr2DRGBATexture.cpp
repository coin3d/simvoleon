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

#include <VolumeViz/render/common/Cvr2DRGBATexture.h>

#include <assert.h>
#include <Inventor/SbName.h>
#include <Inventor/SbVec3s.h>

// *************************************************************************

// Don't set value explicitly to SoType::badType(), to avoid a bug in
// Sun CC v4.0. (Bitpattern 0x0000 equals SoType::badType()).
SoType Cvr2DRGBATexture::classTypeId;

SoType Cvr2DRGBATexture::getTypeId(void) const { return Cvr2DRGBATexture::classTypeId; }
SoType Cvr2DRGBATexture::getClassTypeId(void) { return Cvr2DRGBATexture::classTypeId; }

// *************************************************************************

void
Cvr2DRGBATexture::initClass(void)
{
  assert(Cvr2DRGBATexture::classTypeId == SoType::badType());
  Cvr2DRGBATexture::classTypeId =
    SoType::createType(CvrRGBATexture::getClassTypeId(), "Cvr2DRGBATexture");
}

Cvr2DRGBATexture::Cvr2DRGBATexture(const SbVec3s & size)
  : inherited(size)
{
  assert(Cvr2DRGBATexture::classTypeId != SoType::badType());
}

Cvr2DRGBATexture::~Cvr2DRGBATexture()
{
}

// *************************************************************************

// Returns pointer to RGBA buffer. Allocates memory for it if
// necessary.
uint32_t *
Cvr2DRGBATexture::getRGBABuffer(void) const
{
  if (this->rgbabuffer == NULL) {
    // Cast away constness.
    Cvr2DRGBATexture * that = (Cvr2DRGBATexture *)this;
    const SbVec3s dims = this->getDimensions();
    that->rgbabuffer = new uint32_t[dims[0] * dims[1]];
  }

  return this->rgbabuffer;
}

// Blank out unused texture parts, to make sure we don't get any
// artifacts due to fp-inaccuracies when rendering.
void
Cvr2DRGBATexture::blankUnused(const SbVec3s & texsize) const
{
  assert(this->rgbabuffer);
  assert(texsize[2] == 1);

  const SbVec3s texobjdims = this->getDimensions();
  {
    for (short y=texsize[1]; y < texobjdims[1]; y++) {
      for (short x=0; x < texobjdims[0]; x++) {
        this->rgbabuffer[y * texobjdims[0] + x] = 0x00000000;
      }
    }
  }
  {
    for (short x=texsize[0]; x < texobjdims[0]; x++) {
      for (short y=0; y < texobjdims[1]; y++) {
        this->rgbabuffer[y * texobjdims[0] + x] = 0x00000000;
      }
    }
  }
}

// *************************************************************************
