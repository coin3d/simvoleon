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
  \brief Render the full volume.

  Insert a node of this type after an SoVolumeData node in the scene
  graph to render the full volume data set using raycast rendering via
  OpenCL.

  <b>NB:</b> This node is only available when SIMVoleon is compiled with
  libCLVol support.


  \sa SoVolumeRender, SoOrthoSlice, SoObliqueSlice, SoVolumeFaceSet, SoVolumeIndexedFaceSet
  \sa SoVolumeTriangleStripSet, SoVolumeIndexedTriangleStripSet
*/



#include "SoRaycastRender.h"

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

 
