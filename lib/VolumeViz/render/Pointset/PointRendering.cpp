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

#include <VolumeViz/elements/SoTransferFunctionElement.h>
#include <VolumeViz/elements/SoVolumeDataElement.h>
#include <VolumeViz/misc/CvrCLUT.h>
#include <VolumeViz/misc/CvrUtil.h>
#include <VolumeViz/misc/CvrVoxelChunk.h>
#include <VolumeViz/nodes/SoTransferFunction.h>
#include <VolumeViz/nodes/SoVolumeData.h>
#include <VolumeViz/readers/SoVolumeReader.h>

// *************************************************************************

void
PointRendering::render(SoGLRenderAction * action)
{
  SoState * state = action->getState();

  const SoVolumeDataElement * volumedataelement = SoVolumeDataElement::getInstance(state);
  assert(volumedataelement != NULL);
  SoVolumeData * volumedata = volumedataelement->getVolumeData();
  assert(volumedata != NULL);

  const SoTransferFunctionElement * tfelement = SoTransferFunctionElement::getInstance(state);
  CvrCLUT * clut = CvrVoxelChunk::getCLUT(tfelement);

  SbVec3s dimension;
  void * data;
  SoVolumeData::DataType type;
  
  volumedata->getVolumeData(dimension, data, type);
  // FIXME: support 16-bit data. 20040220 mortene.
  assert(type == SoVolumeData::UNSIGNED_BYTE && "unsupported datatype");
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
