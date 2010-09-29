#ifndef CVR_RAYCASTCUBE_H
#define CVR_RAYCASTCUBE_H

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
#include <Inventor/SbVec3ui32.h>
#include <Inventor/SbVec3f.h>
#include <Inventor/SbBox3f.h>
#include <Inventor/C/glue/gl.h>
#include <Inventor/SbViewportRegion.h>
#include <Inventor/lists/SbList.h>

#include <vector>

class SoGLRenderAction;
class SoState;
class So2DTransferFunction;
class SbViewVolume;
class CvrCLUT;
class CvrRaycastSubcube;
class SbBox3f;

namespace CLVol {
  class RenderManager;
  struct TransferFunctionPoint;
};

class CvrRaycastCube {
public:
  CvrRaycastCube(const SoGLRenderAction * action);
  ~CvrRaycastCube();

  void setTransferFunction(So2DTransferFunction * tf);
  void render(const SoGLRenderAction * action);

private:
  SbVec3s dimensions;
  SbVec3f origo;

  SbVec3ui32 nrsubcubes; // Number of subcubes in x-y-z direction
  SbList<SbBox3f> subcubeboxes;

  CLVol::RenderManager * rendermanager;
  std::vector<GLuint> glcolorlayers;
  std::vector<GLuint> gldepthlayers;
  std::vector<GLuint> gllayerfbos;
  GLuint gltargetfbo;
  GLuint gltargetcolorlayer;
  GLuint gltargetdepthlayer;
  class SubCube ** subcubes;

  std::vector<CLVol::TransferFunctionPoint> transferfunction;
  bool transferfunctionchanged;

  bool reattachglresources;
  SbViewportRegion previousviewportregion;
  uint32_t previoustransferfunctionid;

  void setupRenderManager(const SoGLRenderAction * action);
  void adjustLayers(const SoGLRenderAction * action);
  const SbViewVolume calculateAdjustedViewVolume(const SoGLRenderAction * action) const;
  class SubCube * getSubCube(SoState * state, 
                             unsigned int row, 
                             unsigned int col, 
                             unsigned int depth);
  class SubCube * buildSubCube(const SoGLRenderAction * action,
                               unsigned int row,
                               unsigned int col,
                               unsigned int depth);   
  unsigned int getSubCubeIdx(unsigned int row, unsigned int col, unsigned int depth) const;
  void releaseSubCube(unsigned int row, unsigned int col, unsigned int depth);
  void releaseAllSubCubes();
  float getMostDistantPoint(SbVec3f point, SbBox3f box) const;
};

#endif // !CVR_RAYCASTCUBE_H
