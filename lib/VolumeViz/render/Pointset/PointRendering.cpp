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

// This implements a very simple visualization of the volume data: by
// just plotting a GL point primitive for each voxel.
//
// This is not useful in any real-world situation, as even tiny data
// sets (down to as little as 128^3 voxels) takes seconds to render pr
// frame. It has been implemented just to showcase a skeleton,
// simplest-possible rendering method, e.g. to serve as a minimum
// starting point for adding new rendering methods.
//
// mortene.

// *************************************************************************

#include "PointRendering.h"

#include <Inventor/actions/SoGLRenderAction.h>

#include <VolumeViz/elements/CvrVoxelBlockElement.h>
#include <VolumeViz/elements/SoTransferFunctionElement.h>
#include <VolumeViz/misc/CvrCLUT.h>
#include <VolumeViz/misc/CvrUtil.h>
#include <VolumeViz/misc/CvrVoxelChunk.h>
#include <VolumeViz/nodes/SoTransferFunction.h>

// *************************************************************************

void
PointRendering::render(SoGLRenderAction * action)
{
  SoState * state = action->getState();

  const CvrVoxelBlockElement * vbelem = CvrVoxelBlockElement::getInstance(state);
  assert(vbelem != NULL);

  const SoTransferFunctionElement * tfelement = SoTransferFunctionElement::getInstance(state);
  CvrCLUT * clut = CvrVoxelChunk::getCLUT(tfelement, CvrCLUT::ALPHA_AS_IS);

  const SbVec3s & dimension = vbelem->getVoxelCubeDimensions();
  const void * data = vbelem->getVoxels();
  
  // FIXME: support 16-bit data. 20040220 mortene.
  assert((vbelem->getBytesPrVoxel() == 1) && "unsupported datatype");
  const uint8_t * voxels = (const uint8_t *)data;

  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_TEXTURE_3D);

  // FIXME: support the special rendering modes "max intensity" and
  // "sum intensity". 20040222 mortene.

  glBegin(GL_POINTS);

  const unsigned int XYPAGEWIDTH = (unsigned int)dimension[0];
  const unsigned int XYPAGEHEIGHT = (unsigned int)dimension[1];
  const unsigned int STACKDEPTH = (unsigned int)dimension[2];
  const unsigned int XYPAGESIZE = XYPAGEWIDTH * XYPAGEHEIGHT;

  // FIXME: support the numslices setting. 20040222 mortene.
  // FIXME: support the abort callback from the public API. 20040222 mortene.
  for (unsigned int z=0; z < STACKDEPTH; z++) {
    const unsigned int CURRENTDEPTH = z * XYPAGESIZE;
    // FIXME: the y-axis is rendered upside down versus 2D texture
    // rendering -- which one is correct? 20040222 mortene.
    for (unsigned int y=0; y < XYPAGEHEIGHT; y++) {
      const unsigned int CURRENTPAGEPOSITION = y * XYPAGEWIDTH;
      for (unsigned int x=0; x < XYPAGEWIDTH; x++) {
        uint8_t colidx = voxels[CURRENTDEPTH + CURRENTPAGEPOSITION + x];
        uint8_t rgba[4];
        clut->lookupRGBA(colidx, rgba);
        if (rgba[3] > 0x00) {
          glColor4ubv(rgba);
          // FIXME: heed the volume object size settings. 20040222 mortene.
          glVertex3f(x - XYPAGEWIDTH / 2.0f,
                     y - XYPAGEHEIGHT / 2.0f,
                     z - STACKDEPTH / 2.0f);
        }
      }
    }
  }

  glEnd();

  glPopAttrib();
}

// *************************************************************************
