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

#include <VolumeViz/render/common/Cvr3DPaletteGradientTexture.h>

#include <assert.h>
#include <stdlib.h>
#include <Inventor/SbName.h>
#include <VolumeViz/misc/CvrCLUT.h>

// *************************************************************************

// Don't set value explicitly to SoType::badType(), to avoid a bug in
// Sun CC v4.0. (Bitpattern 0x0000 equals SoType::badType()).
SoType Cvr3DPaletteGradientTexture::classTypeId;

SoType Cvr3DPaletteGradientTexture::getTypeId(void) const { return Cvr3DPaletteGradientTexture::classTypeId; }
SoType Cvr3DPaletteGradientTexture::getClassTypeId(void) { return Cvr3DPaletteGradientTexture::classTypeId; }

void * Cvr3DPaletteGradientTexture::createInstance(void) { return new Cvr3DPaletteGradientTexture; }

// *************************************************************************

void
Cvr3DPaletteGradientTexture::initClass(void)
{
  assert(Cvr3DPaletteGradientTexture::classTypeId == SoType::badType());
  Cvr3DPaletteGradientTexture::classTypeId =
    SoType::createType(Cvr3DPaletteTexture::getClassTypeId(),
                       "Cvr3DPaletteGradientTexture",
                       Cvr3DPaletteGradientTexture::createInstance);
}

Cvr3DPaletteGradientTexture::Cvr3DPaletteGradientTexture(void)
{
  assert(Cvr3DPaletteGradientTexture::classTypeId != SoType::badType());
}

Cvr3DPaletteGradientTexture::~Cvr3DPaletteGradientTexture()
{
}

// *************************************************************************

// Returns pointer to buffer with 32-bit indices. Allocates memory for
// it if necessary.
uint8_t *
Cvr3DPaletteGradientTexture::getIndex8Buffer(void) const
{
  if (this->indexbuffer == NULL) {
    // Cast away constness.
    Cvr3DPaletteGradientTexture * that = (Cvr3DPaletteGradientTexture *)this;
    const SbVec3s dims = this->getDimensions();
    that->indexbuffer = (uint8_t *) calloc(dims[0] * dims[1] * dims[2], sizeof(uint8_t) * 4);
    //that->indexbuffer = new uint8_t[dims[0] * dims[1] * dims[2] * 4];
    //for (int i=0; i < dims[0] * dims[1] * dims[2] * 4; i++) that->indexbuffer[i] = 0;
  }

  return this->indexbuffer;
}

// Empty since we use calloc to clear mem
void
Cvr3DPaletteGradientTexture::blankUnused(const SbVec3s & texsize) const
{

}

// *************************************************************************
