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

/*!
  \class SoVolumeIndexedTriangleStripSet VolumeViz/nodes/SoVolumeIndexedTriangleStripSet.h
  \brief Render a set of trianglestrips within the volume.

  This node works like the SoVolumeTriangleStripSet node, but
  specifies vertex indices in a slightly different manner. See
  documentation of SoVolumeTriangleStripSet, SoVolumeFaceSet and
  Coin's SoIndexedTriangleStripSet for further information.

  Note that this node will not work with OpenGL drivers too old to
  contain support for 3D-texturing. See the extended comments on
  SoObliqueSlice for more information.

  \sa SoVolumeIndexedFaceSet, SoVolumeRender, SoOrthoSlice, SoObliqueSlice
  \sa SoVolumeTriangleStripSet, SoVolumeFaceSet

  \since SIM Voleon 1.1
*/

// *************************************************************************

#include <VolumeViz/nodes/SoVolumeIndexedTriangleStripSet.h>

#include <Inventor/C/tidbits.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoGLLazyElement.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoCoordinateElement.h>
#include <Inventor/elements/SoClipPlaneElement.h>
#include <Inventor/elements/SoTextureQualityElement.h>

#include <VolumeViz/elements/SoTransferFunctionElement.h>
#include <VolumeViz/misc/CvrCLUT.h>
#include <VolumeViz/misc/CvrVoxelChunk.h>
#include <VolumeViz/misc/CvrUtil.h>

#include "CvrIndexedTriangleStripSetRenderP.h"

// *************************************************************************

SO_NODE_SOURCE(SoVolumeIndexedTriangleStripSet);

// *************************************************************************

class SoVolumeIndexedTriangleStripSetP {
public:
  SoVolumeIndexedTriangleStripSetP(SoVolumeIndexedTriangleStripSet * master)
  {
    this->master = master; 
    this->renderp = new CvrIndexedTriangleStripSetRenderP(master);
    this->renderp->clipgeometryshape = new SoIndexedTriangleStripSet;
    this->renderp->clipgeometryshape->ref();   
  }

  ~SoVolumeIndexedTriangleStripSetP() {
    this->renderp->clipgeometryshape->unref();
    delete this->renderp;
  }
  CvrIndexedTriangleStripSetRenderP * renderp;

private:
  SoVolumeIndexedTriangleStripSet * master;
};

#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

// *************************************************************************

SoVolumeIndexedTriangleStripSet::SoVolumeIndexedTriangleStripSet(void)
{
  SO_NODE_CONSTRUCTOR(SoVolumeIndexedTriangleStripSet);
  PRIVATE(this) = new SoVolumeIndexedTriangleStripSetP(this);

  SO_NODE_ADD_FIELD(clipGeometry, (FALSE));
  SO_NODE_ADD_FIELD(offset, (0.0f)); 
}

SoVolumeIndexedTriangleStripSet::~SoVolumeIndexedTriangleStripSet(void)
{
  delete PRIVATE(this);
}

// Doc from parent class.
void
SoVolumeIndexedTriangleStripSet::initClass(void)
{
  SO_NODE_INIT_CLASS(SoVolumeIndexedTriangleStripSet, SoIndexedTriangleStripSet, "SoIndexedTriangleStripSet");
}

void
SoVolumeIndexedTriangleStripSet::GLRender(SoGLRenderAction * action)
{

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

  PRIVATE(this)->renderp->GLRender(action, this->offset.getValue(), this->clipGeometry.getValue());

}
