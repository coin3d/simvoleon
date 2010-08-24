/**************************************************************************\
 *
 *  This file is part of the SIM Voleon visualization library.
 *  Copyright (C) by Kongsberg Oil & Gas Technologies.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  ("GPL") version 2 as published by the Free Software Foundation.
 *  See the file LICENSE.GPL at the root directory of this source
 *  distribution for additional information about the GNU GPL.
 *
 *  For using SIM Voleon with software that can not be combined with
 *  the GNU GPL, and for taking advantage of the additional benefits
 *  of our support services, please contact Kongsberg Oil & Gas
 *  Technologies about acquiring a SIM Voleon Professional Edition
 *  License.
 *
 *  See http://www.coin3d.org/ for more information.
 *
 *  Kongsberg Oil & Gas Technologies, Bygdoy Alle 5, 0257 Oslo, NORWAY.
 *  http://www.sim.no/  sales@sim.no  coin-support@coin3d.org
 *
\**************************************************************************/

/*!
  \class SoRaycastRender VolumeViz/nodes/SoRaycastRender.h
  \brief Render the full volume using raycasting instead of the
  traditional slicing.

  Insert a node of this type after an SoVolumeData node in the scene
  graph to render the full volume data set using raycast rendering
  through OpenCL.

  <b>NB:</b> This node is only available when SIMVoleon is compiled with
  libCLVol support.


  \sa SoVolumeRender, SoOrthoSlice, SoObliqueSlice, SoVolumeFaceSet, SoVolumeIndexedFaceSet
  \sa SoVolumeTriangleStripSet, SoVolumeIndexedTriangleStripSet
*/

#include "SoRaycastRender.h"

#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/actions/SoRayPickAction.h>
#include <Inventor/elements/SoLazyElement.h>
#include <Inventor/elements/SoModelMatrixElement.h>

#include <VolumeViz/elements/SoTransferFunctionElement.h>

#include <VolumeViz/render/raycast/CvrRaycastCube.h>

// *************************************************************************

SO_NODE_SOURCE(SoRaycastRender);

// *************************************************************************


class SoRaycastRenderP {
public:
  SoRaycastRenderP(SoRaycastRender * master) {
    this->master = master;
  }
  SoRaycastRender * master;
};

#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)


// *************************************************************************


SoRaycastRender::SoRaycastRender()
{
}


SoRaycastRender::~SoRaycastRender()
{
}


void
SoRaycastRender::initClass(void)
{
  SO_NODE_INIT_CLASS(SoRaycastRender, SoShape, "SoShape");

  SO_ENABLE(SoGLRenderAction, SoTransferFunctionElement);
  SO_ENABLE(SoGLRenderAction, SoModelMatrixElement);
  SO_ENABLE(SoGLRenderAction, SoLazyElement);

  SO_ENABLE(SoRayPickAction, SoTransferFunctionElement);
  SO_ENABLE(SoRayPickAction, SoModelMatrixElement);
}


void 
SoRaycastRender::GLRender(SoGLRenderAction * action)
{
}


void 
SoRaycastRender::rayPick(SoRayPickAction * action)
{
}


void 
SoRaycastRender::generatePrimitives(SoAction * action)
{
}


void 
SoRaycastRender::computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center)
{
}

 
