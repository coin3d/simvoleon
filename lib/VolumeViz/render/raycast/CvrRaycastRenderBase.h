#ifndef CVR_RAYCASTRENDERBASE_H
#define CVR_RAYCASTRENDERBASE_H

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
#include <Inventor/C/glue/gl.h>
#include <Inventor/SbBox3f.h>
#include <Inventor/C/glue/gl.h>
#include <Inventor/SbViewportRegion.h>
#include <Inventor/lists/SbList.h>

#include "CvrRaycastSubCube.h"

#include <vector>

class SoGLRenderAction;
class SoState;
class So2DTransferFunction;
class CvrCLUT;
class SbBox3f;

namespace CLVol {
  class RenderManager;
  class TransferFunction;
  struct TransferFunctionPoint;
};

class SubCube {
public:
  SubCube(CvrRaycastSubCube * p) { this->cube = p; }
  CvrRaycastSubCube * cube;
  SbBox3f bbox;
  double distancefromcamera;
  uint32_t volumedataid;
};


class CvrRaycastRenderBase {
public:
  CvrRaycastRenderBase(const SoGLRenderAction * action);
  ~CvrRaycastRenderBase();
  void setTransferFunction(So2DTransferFunction * tf);
  virtual void render(SoGLRenderAction * action) = 0;

protected:
  CLVol::RenderManager * getRenderManager(const SoGLRenderAction * action);
  SbList<SubCube *> processSubCubes(const SoGLRenderAction * action);

  SbVec3ui32 getNumberOfSubcubes() const;
  class SubCube * getSubCube(SoState * state, 
                             unsigned int row, 
                             unsigned int col, 
                             unsigned int depth);
 
  // FIXME: Hide this by adding get'ers instead. (20101105 handegar)
  std::vector<GLuint> gllayerfbos;


private:
  SbVec3s dimensions;
  SbVec3f origo;
  SbVec3ui32 nrsubcubes; // Number of subcubes in x-y-z direction
  SbList<SbBox3f> subcubeboxes;

  std::vector<GLuint> glcolorlayers;
  std::vector<GLuint> gldepthlayers;
  GLuint gltargetfbo;
  GLuint gltargetcolorlayer;
  GLuint gltargetdepthlayer;

  static CLVol::RenderManager * rendermanager;

  class SubCube ** subcubes;

  std::vector<float> transferfunctionpoints;
  bool transferfunctionchanged;
  uint32_t previoustransferfunctionid;
  CLVol::TransferFunction * cltransferfunction;

  SbViewportRegion previousviewportregion;

  void setupRenderManager(const SoGLRenderAction * action);
  void adjustLayers(const SoGLRenderAction * action);

  class SubCube * buildSubCube(const SoGLRenderAction * action,
                               unsigned int row,
                               unsigned int col,
                               unsigned int depth);   
  unsigned int getSubCubeIdx(unsigned int row, 
                             unsigned int col, 
                             unsigned int depth) const;
  void releaseSubCube(unsigned int row, 
                      unsigned int col, 
                      unsigned int depth);
  void releaseAllSubCubes();
  void sortSubCubes(SbList<SubCube *> & subcubelist) const;
};


#endif // !CVR_RAYCASTRENDERBASE_H
