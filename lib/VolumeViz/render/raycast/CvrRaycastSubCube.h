#ifndef CVR_RAYCASTSUBCUBE_H
#define CVR_RAYCASTSUBCUBE_H

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

#ifndef SIMVOLEON_INTERNAL
#error this is a private header file
#endif // !SIMVOLEON_INTERNAL

#include <Inventor/SbVec3s.h>
#include <Inventor/SbVec3f.h>
#include <Inventor/SbVec2f.h>
#include <Inventor/SbBox3s.h>
#include <Inventor/C/glue/gl.h>

#include <vector>


class SoGLRenderAction;
class CvrRaycastTexture;
class SbViewVolume;
class SbMatrix;

namespace CLVol {
  class RenderManager;
  struct TransferFunctionPoint;
}

class CvrRaycastSubCube {
public:
  CvrRaycastSubCube(const SoGLRenderAction * action,
                    const CvrRaycastTexture * texobj,
                    const SbBox3s cubebbox,
                    const SbVec3s totalsize,
                    CLVol::RenderManager * rm);
  ~CvrRaycastSubCube(); 

  void renderVolume(const SoGLRenderAction * action);
  void renderFaceset(const SoGLRenderAction * action);

private:
  void prepareForRendering(const SoGLRenderAction * action,
                           SbMatrix & P, SbMatrix & PMi, 
                           std::vector<GLfloat> & clipplanes);

  const CvrRaycastTexture * textureobject;
  SbBox3s bbox;
  SbVec3s totalsize;
  CLVol::RenderManager * rendermanager;
};

#endif // !CVR_RAYCASTSUBCUBE_H
