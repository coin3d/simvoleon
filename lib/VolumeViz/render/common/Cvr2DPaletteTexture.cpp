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

#include <VolumeViz/render/common/Cvr2DPaletteTexture.h>

#include <assert.h>
#include <Inventor/SbName.h>
#include <Inventor/SbVec3s.h>
#include <VolumeViz/misc/CvrCLUT.h>

// *************************************************************************

// Don't set value explicitly to SoType::badType(), to avoid a bug in
// Sun CC v4.0. (Bitpattern 0x0000 equals SoType::badType()).
SoType Cvr2DPaletteTexture::classTypeId;

SoType Cvr2DPaletteTexture::getTypeId(void) const { return Cvr2DPaletteTexture::classTypeId; }
SoType Cvr2DPaletteTexture::getClassTypeId(void) { return Cvr2DPaletteTexture::classTypeId; }

// *************************************************************************

void
Cvr2DPaletteTexture::initClass(void)
{
  assert(Cvr2DPaletteTexture::classTypeId == SoType::badType());
  Cvr2DPaletteTexture::classTypeId =
    SoType::createType(CvrTextureObject::getClassTypeId(), "Cvr2DPaletteTexture");
}

Cvr2DPaletteTexture::Cvr2DPaletteTexture(const SbVec3s & size)
  : inherited(size)
{
  assert(Cvr2DPaletteTexture::classTypeId != SoType::badType());
  assert(size[2] == 1);
}

Cvr2DPaletteTexture::~Cvr2DPaletteTexture()
{
}

// *************************************************************************

// Returns pointer to buffer with 8-bit indices. Allocates memory for
// it if necessary.
uint8_t *
Cvr2DPaletteTexture::getIndex8Buffer(void) const
{
  if (this->indexbuffer == NULL) {
    // Cast away constness.
    Cvr2DPaletteTexture * that = (Cvr2DPaletteTexture *)this;
    const SbVec3s dim = this->getDimensions();
    that->indexbuffer = new uint8_t[dim[0] * dim[1]];
  }

  return this->indexbuffer;
}

// Blank out unused texture parts, to make sure we don't get any
// artifacts due to fp-inaccuracies when rendering.
void
Cvr2DPaletteTexture::blankUnused(const SbVec3s & texsize) const
{
  assert(this->indexbuffer);
  assert(texsize[2] == 1);
  
  const SbVec3s texobjdims = this->getDimensions();
  {
    for (short y=texsize[1]; y < texobjdims[1]; y++) {
      for (short x=0; x < texobjdims[0]; x++) {
        this->indexbuffer[y * texobjdims[0] + x] = 0x00;
      }
    }
  }
  {
    for (short x=texsize[0]; x < texobjdims[0]; x++) {
      for (short y=0; y < texobjdims[1]; y++) {
        this->indexbuffer[y * texobjdims[0] + x] = 0x00;
      }
    }
  }

}


void
Cvr2DPaletteTexture::dumpToPPM(const char * filename) const
{
  FILE * f = fopen(filename, "w");
  assert(f);

  const SbVec3s texobjdims = this->getDimensions();
  // width height maxcolval
  (void)fprintf(f, "P3\n%d %d 255\n", texobjdims[0], texobjdims[1]);

  for (int i=0; i < texobjdims[0] * texobjdims[1]; i++) {
#if 0
    uint8_t index = this->indexbuffer[i];
    // FIXME: convert index value to rgba
    fprintf(f, "%d %d %d\n",
            rgba & 0xff, (rgba & 0xff00) >> 8,  (rgba & 0xff0000) >> 16);
#else
    assert(FALSE && "yet unimplemented");
#endif
  }
  fclose(f);
}

// *************************************************************************
