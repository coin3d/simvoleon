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

/*!
  \class SoVolumeTriangleStripSet VolumeViz/nodes/SoVolumeTriangleStripSet.h
  \brief Render a set of trianglestrips within the volume.

  This node works like the SoVolumeFaceSet node, but specifies
  polygons in triangle strips instead of individual polygons. See
  documentation of SoVolumeFaceSet and Coin's SoTriangleStripSet for
  further information.

  Note that this node will not work with OpenGL drivers too old to
  contain support for 3D-texturing. See the extended comments on
  SoObliqueSlice for more information.

  \sa SoVolumeIndexedFaceSet, SoVolumeRender, SoOrthoSlice, SoObliqueSlice
  \sa SoVolumeIndexedTriangleStripSet, SoVolumeFaceSet

  \since SIM Voleon 2.0
*/

// *************************************************************************

#include <VolumeViz/nodes/SoVolumeTriangleStripSet.h>

#include <Inventor/C/tidbits.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoGLLazyElement.h>
#include <Inventor/bundles/SoMaterialBundle.h>
#include <Inventor/bundles/SoTextureCoordinateBundle.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoCoordinateElement.h>
#include <Inventor/elements/SoClipPlaneElement.h>
#include <Inventor/elements/SoTextureQualityElement.h>

#include <VolumeViz/elements/CvrGLInterpolationElement.h>
#include <VolumeViz/elements/SoTransferFunctionElement.h>
#include <VolumeViz/misc/CvrCLUT.h>
#include <VolumeViz/misc/CvrVoxelChunk.h>
#include <VolumeViz/misc/CvrUtil.h>
#include <VolumeViz/misc/CvrGlobalRenderLock.h>

#include "CvrTriangleStripSetRenderP.h"

// *************************************************************************

SO_NODE_SOURCE(SoVolumeTriangleStripSet);

// *************************************************************************

class SoVolumeTriangleStripSetP {
public:
  SoVolumeTriangleStripSetP(SoVolumeTriangleStripSet * master)
  {
    this->master = master;    
    this->renderp = new CvrTriangleStripSetRenderP(master);
    this->renderp->clipgeometryshape = new SoTriangleStripSet;
    this->renderp->clipgeometryshape->ref();
  }
  ~SoVolumeTriangleStripSetP()
  {
    this->renderp->clipgeometryshape->unref();
    delete this->renderp;
  }
 
  CvrTriangleStripSetRenderP * renderp;

private:
  SoVolumeTriangleStripSet * master;
};

#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

// *************************************************************************

SoVolumeTriangleStripSet::SoVolumeTriangleStripSet(void)
{
  SO_NODE_CONSTRUCTOR(SoVolumeTriangleStripSet);
  PRIVATE(this) = new SoVolumeTriangleStripSetP(this);

  SO_NODE_ADD_FIELD(clipGeometry, (FALSE));
  SO_NODE_ADD_FIELD(offset, (0.0f));
}

SoVolumeTriangleStripSet::~SoVolumeTriangleStripSet(void)
{  
  delete PRIVATE(this);
}

// Doc from parent class.
void
SoVolumeTriangleStripSet::initClass(void)
{
  SO_NODE_INIT_CLASS(SoVolumeTriangleStripSet, SoTriangleStripSet, "SoTriangleStripSet");

  SO_ENABLE(SoGLRenderAction, CvrGLInterpolationElement);
}

void
SoVolumeTriangleStripSet::GLRender(SoGLRenderAction * action)
{
  // This will automatically lock and unlock a mutex stopping multiple
  // render threads for SIM Voleon nodes. FIXME: should really make
  // code re-entrant / threadsafe. 20041112 mortene.
  CvrGlobalRenderLock lock;


  // FIXME: need to make sure we're not cached in a renderlist
  if (!this->shouldGLRender(action)) return;

  // Render at the end, in case the volume is partly (or fully)
  // transparent.
  //
  // FIXME: this makes rendering a bit slower, so we should perhaps
  // keep a flag around to know whether or not this is actually
  // necessary. 20040212 mortene.
  
  if (!action->isRenderingDelayedPaths()) {
    action->addDelayedPath(action->getCurPath()->copy());
    return;
  }
  
  PRIVATE(this)->renderp->GLRender(action, 
                                   this->offset.getValue(), 
                                   this->clipGeometry.getValue(),
                                   this->numVertices);
}

