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

#include <VolumeViz/render/common/Cvr3DPaletteTexture.h>
#include <VolumeViz/misc/CvrCLUT.h>
#include <Inventor/SbName.h>
#include <assert.h>

// Don't set value explicitly to SoType::badType(), to avoid a bug in
// Sun CC v4.0. (Bitpattern 0x0000 equals SoType::badType()).
SoType Cvr3DPaletteTexture::classTypeId;

SoType Cvr3DPaletteTexture::getTypeId(void) const { return Cvr3DPaletteTexture::classTypeId; }
SoType Cvr3DPaletteTexture::getClassTypeId(void) { return Cvr3DPaletteTexture::classTypeId; }

void
Cvr3DPaletteTexture::initClass(void)
{
  assert(Cvr3DPaletteTexture::classTypeId == SoType::badType());
  Cvr3DPaletteTexture::classTypeId =
    SoType::createType(CvrTextureObject::getClassTypeId(), "Cvr3DPaletteTexture");
}

Cvr3DPaletteTexture::Cvr3DPaletteTexture(const SbVec3s & size)
{
  assert(Cvr3DPaletteTexture::classTypeId != SoType::badType());
  this->dimensions = size;
}

Cvr3DPaletteTexture::~Cvr3DPaletteTexture()
{
}

// Returns pointer to buffer with 8-bit indices. Allocates memory for
// it if necessary.
uint8_t *
Cvr3DPaletteTexture::getIndex8Buffer(void) const
{
  if (this->indexbuffer == NULL) {
    // Cast away constness.
    Cvr3DPaletteTexture * that = (Cvr3DPaletteTexture *)this;
    that->indexbuffer = new uint8_t[this->dimensions[0] * this->dimensions[1] * this->dimensions[2]];
  }

  return this->indexbuffer;
}

// Blank out unused texture parts, to make sure we don't get any
// artifacts due to fp-inaccuracies when rendering.
void
Cvr3DPaletteTexture::blankUnused(const SbVec3s & texsize) const
{
  assert(this->indexbuffer);
 
  for (short z=texsize[2]; z < this->dimensions[2]; z++) {    
    for (short y=texsize[1]; y < this->dimensions[1]; y++) {
      for (short x=0; x < this->dimensions[0]; x++) {
        this->indexbuffer[(z * this->dimensions[0] * this->dimensions[1]) + 
                          (y * this->dimensions[0]) + x] = 0x00;
      }
    }        
    for (short x=texsize[0]; x < this->dimensions[0]; x++) {
      for (short y=0; y < this->dimensions[1]; y++) {
        this->indexbuffer[(z * this->dimensions[0] * this->dimensions[1]) + 
                          (y * this->dimensions[0]) + x] = 0x00;
      }
    }    
  }
  
}
