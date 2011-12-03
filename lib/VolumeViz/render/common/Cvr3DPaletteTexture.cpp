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
    that->indexbuffer = new uint8_t[dims[0] * dims[1] * dims[2]];
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
