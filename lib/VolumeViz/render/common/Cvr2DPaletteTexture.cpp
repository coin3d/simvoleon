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

#include <VolumeViz/render/common/Cvr2DPaletteTexture.h>

#include <assert.h>
#include <string.h>
#include <Inventor/SbName.h>
#include <Inventor/SbVec3s.h>
#include <VolumeViz/misc/CvrCLUT.h>

// *************************************************************************

// Don't set value explicitly to SoType::badType(), to avoid a bug in
// Sun CC v4.0. (Bitpattern 0x0000 equals SoType::badType()).
SoType Cvr2DPaletteTexture::classTypeId;

SoType Cvr2DPaletteTexture::getTypeId(void) const { return Cvr2DPaletteTexture::classTypeId; }
SoType Cvr2DPaletteTexture::getClassTypeId(void) { return Cvr2DPaletteTexture::classTypeId; }

void * Cvr2DPaletteTexture::createInstance(void) { return new Cvr2DPaletteTexture; }

// *************************************************************************

void
Cvr2DPaletteTexture::initClass(void)
{
  assert(Cvr2DPaletteTexture::classTypeId == SoType::badType());
  Cvr2DPaletteTexture::classTypeId =
    SoType::createType(CvrPaletteTexture::getClassTypeId(),
                       "Cvr2DPaletteTexture",
                       Cvr2DPaletteTexture::createInstance);
}

Cvr2DPaletteTexture::Cvr2DPaletteTexture(void)
{
  assert(Cvr2DPaletteTexture::classTypeId != SoType::badType());
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
    // adding 2 two each dimension to make room for border 20090408 eigils
    //
    // FIXME: i don't quite understand why it is necessary to add in
    // the extra 2 pixels for border seams here, as the value from
    // getDimensions() already has been justified with that +2. we get
    // a crash from the OpenGL driver if left out, however, so it is
    // obviously needed (the crash seems to happen where a texture is
    // created from this memory buffer). should investigate. 20090812 mortene.
    const size_t bufsize = (dim[0]+2) * (dim[1]+2);
    that->indexbuffer = new uint8_t[bufsize];
    // FIXME: suddenly, this was also needed, which was not the case
    // previously. should investigate why. 20090813 mortene.
    (void)memset(that->indexbuffer, 0, bufsize);

    // FIXME: to reproduce the problem seen without the memset(),
    // enable the code below to produce a distinct pattern, then run
    // the minimal example from the SIM Voleon Doxygen main page, but
    // with DIM=29 (set near the top). 20090813 mortene.
    //
    // for (size_t i = 0; i < bufsize; i++) { that->indexbuffer[i] = i % 0xff; }
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
    for (short y=texsize[1]+2; y < texobjdims[1]+2; y++) {
      for (short x=0; x < texobjdims[0]+2; x++) {
        this->indexbuffer[y * (texobjdims[0]+2) + x] = 0x00;
      }
    }
  }
  {
    for (short x=texsize[0]+2; x < texobjdims[0]+2; x++) {
      for (short y=0; y < texobjdims[1]+2; y++) {
        this->indexbuffer[y * (texobjdims[0]+2) + x] = 0x00;
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
