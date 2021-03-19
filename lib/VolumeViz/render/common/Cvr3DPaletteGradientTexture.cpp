/**************************************************************************\
 * Copyright (c) Kongsberg Oil & Gas Technologies AS
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
    // FIXME: what is calloc()'ed here is probably delete'd somewhere
    // else, which is not good. Fix. 20050628 mortene.
    that->indexbuffer = (uint8_t *) calloc((size_t)dims[0] * dims[1] * dims[2], sizeof(uint8_t) * 4);
    //that->indexbuffer = new uint8_t[dims[0] * dims[1] * dims[2] * 4];
    //for (int i=0; i < dims[0] * dims[1] * dims[2] * 4; i++) that->indexbuffer[i] = 0;
  }

  return this->indexbuffer;
}

// Empty since we use calloc to clear mem
// FIXME: get rid of this? 20050628 mortene.
void
Cvr3DPaletteGradientTexture::blankUnused(const SbVec3s & texsize) const
{

}

// *************************************************************************
