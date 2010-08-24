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
 *  Systems in Motion, Postboks 1283, Pirsenteret, 7462 Trondheim, NORWAY.
 *  <URL:http://www.sim.no/>.
 *
\**************************************************************************/

#include "CvrRaycastSubCube.h"
#include <VolumeViz/render/common/CvrTextureObject.h>


CvrRaycastSubCube::CvrRaycastSubCube(const SoGLRenderAction * action,
                                     const CvrTextureObject * texobj,
                                     const SbVec3f & cubeorigo,
                                     const SbVec3s & cubesize)
  : dimensions(cubesize), origo(cubeorigo), textureobject(texobj)
{
  this->textureobject->ref();

}


CvrRaycastSubCube::~CvrRaycastSubCube()
{
  this->textureobject->unref();
}


void 
CvrRaycastSubCube::render(const SoGLRenderAction * action)
{
  // **
  // FIXME: Trigger a rendering using libCLVol here (20100824 handegar)
  // **

  /*
    Todo:
    * share textureobject with libCLVol
    * share current modelmatrix with libCLVol
    * trigger libCLVol rendering

    All rendering will be done to the same framebuffer in libCLVol
    activated by CvrRaycastCube.

   */

}



