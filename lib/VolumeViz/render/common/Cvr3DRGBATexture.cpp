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
