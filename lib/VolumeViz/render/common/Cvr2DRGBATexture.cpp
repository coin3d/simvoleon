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

#include <VolumeViz/render/common/Cvr2DRGBATexture.h>

#include <assert.h>
#include <string.h>
#include <Inventor/SbName.h>
#include <Inventor/SbVec3s.h>

// *************************************************************************

// Don't set value explicitly to SoType::badType(), to avoid a bug in
// Sun CC v4.0. (Bitpattern 0x0000 equals SoType::badType()).
SoType Cvr2DRGBATexture::classTypeId;

SoType Cvr2DRGBATexture::getTypeId(void) const { return Cvr2DRGBATexture::classTypeId; }
SoType Cvr2DRGBATexture::getClassTypeId(void) { return Cvr2DRGBATexture::classTypeId; }

void * Cvr2DRGBATexture::createInstance(void) { return new Cvr2DRGBATexture; }

// *************************************************************************

void
Cvr2DRGBATexture::initClass(void)
{
  assert(Cvr2DRGBATexture::classTypeId == SoType::badType());
  Cvr2DRGBATexture::classTypeId =
    SoType::createType(CvrRGBATexture::getClassTypeId(), "Cvr2DRGBATexture",
                       Cvr2DRGBATexture::createInstance);
}

Cvr2DRGBATexture::Cvr2DRGBATexture(void)
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
    //adding 2 two each dimension to make room for border 20090408 eigils
    //
    // FIXME: i believe this shouldn't really be necessary, see
    // discussion in the FIXME in
    // Cvr2DPaletteTexture::getIndex8Buffer(). 20090812 mortene.
    that->rgbabuffer = new uint32_t[(dims[0]+2) * (dims[1]+2)];

    // FIXME: suddenly, this was also needed, which was not the case
    // previously. should investigate why. see FIXME in
    // Cvr2DPaletteTexture::getIndex8Buffer() for info on how to
    // reproduce. 20090813 mortene.
    (void)memset(that->rgbabuffer, 0, (dims[0]+2) * (dims[1]+2) * sizeof(uint32_t));
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
    for (short y=texsize[1]+2; y < texobjdims[1]+2; y++) {
      for (short x=0; x < texobjdims[0]+2; x++) {
        this->rgbabuffer[y * (texobjdims[0]+2) + x] = 0x00000000;
      }
    }
  }
  {
    for (short x=texsize[0]+2; x < texobjdims[0]+2; x++) {
      for (short y=0; y < texobjdims[1]+2; y++) {
        this->rgbabuffer[y * (texobjdims[0]+2) + x] = 0x00000000;
      }
    }
  }
}

// *************************************************************************
