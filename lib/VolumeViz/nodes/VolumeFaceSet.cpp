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
  \class SoVolumeFaceSet VolumeViz/nodes/SoVolumeFaceSet.h
  \brief Render a set of faces within the volume.

  This node works somewhat like the SoObliqueSlice node, but instead
  of a single plane cutting through the full volume, one can set up an
  arbitrary number of polygon faces inside (and outside) the volume,
  where the voxel values which the polygons are cutting through will
  be shown mapped onto the faces.

  Coordinates, materials, normals, and other settings for the polygons
  should be specified as for the Coin SoFaceSet node, so see class
  documentation of that class for more information.

  Note that this node will not work with OpenGL drivers too old to
  contain support for 3D-texturing. See the extended comments on
  SoObliqueSlice for more information.

  \sa SoVolumeIndexedFaceSet, SoVolumeRender, SoOrthoSlice, SoObliqueSlice
  \sa SoVolumeTriangleStripSet, SoVolumeIndexedTriangleStripSet

  \since SIM Voleon 2.0
*/

// *************************************************************************

#include <VolumeViz/nodes/SoVolumeFaceSet.h>

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

#include "CvrFaceSetRenderP.h"

// *************************************************************************

SO_NODE_SOURCE(SoVolumeFaceSet);

// *************************************************************************

class SoVolumeFaceSetP {
public:
  SoVolumeFaceSetP(SoVolumeFaceSet * master)
  {
    this->master = master; 
    this->renderp = new CvrFaceSetRenderP(master);
    this->renderp->clipgeometryshape = new SoFaceSet;
    this->renderp->clipgeometryshape->ref();
  }
  ~SoVolumeFaceSetP()
  {
    this->renderp->clipgeometryshape->unref();
    delete this->renderp;
  } 
  CvrFaceSetRenderP * renderp;

private:
  SoVolumeFaceSet * master;
};

#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

// *************************************************************************

SoVolumeFaceSet::SoVolumeFaceSet(void)
{
  SO_NODE_CONSTRUCTOR(SoVolumeFaceSet);
  PRIVATE(this) = new SoVolumeFaceSetP(this);
  SO_NODE_ADD_FIELD(clipGeometry, (FALSE));
  SO_NODE_ADD_FIELD(offset, (0.0f));
}

SoVolumeFaceSet::~SoVolumeFaceSet(void)
{  
  delete PRIVATE(this);
}

// Doc from parent class.
void
SoVolumeFaceSet::initClass(void)
{
  SO_NODE_INIT_CLASS(SoVolumeFaceSet, SoFaceSet, "SoFaceSet");

  SO_ENABLE(SoGLRenderAction, CvrGLInterpolationElement);
}

void
SoVolumeFaceSet::GLRender(SoGLRenderAction * action)
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
  
  PRIVATE(this)->renderp->GLRender(action, 
                                   this->offset.getValue(), 
                                   this->clipGeometry.getValue(),
                                   this->numVertices);
  
}


