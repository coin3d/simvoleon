#ifndef COIN_CVRCUBEHANDLER_H
#define COIN_CVRCUBEHANDLER_H

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

#include <Inventor/SbBox2f.h>
#include <Inventor/SbVec3f.h>
#include <Inventor/SbVec3s.h>
#include <VolumeViz/render/3D/Cvr3DTexSubCube.h>
#include <VolumeViz/nodes/SoVolumeRender.h>

class Cvr3DTexCube;
class SoVolumeReader;
class SoVolumeData;
class SoState;
class SoGLRenderAction;


class CvrCubeHandler {
public:
  CvrCubeHandler(const SbVec3s & voldatadims, SoVolumeReader * reader);
  ~CvrCubeHandler();

  enum Composition { MAX_INTENSITY, SUM_INTENSITY, ALPHA_BLENDING };

  void render(SoGLRenderAction * action, unsigned int numslices,
              Cvr3DTexSubCube::Interpolation interpolation,
              CvrCubeHandler::Composition composition,
              SoVolumeRender::SoVolumeRenderAbortCB * abortfunc,
              void * abortcbdata);

  unsigned int getCurrentAxis(SoGLRenderAction * action) const;

  void releaseAllSlices(void);
  void releaseSlices(const unsigned int AXISIDX);

private:
  unsigned int getCurrentAxis(const SbVec3f & viewvec) const;
  void getViewVector(SoGLRenderAction * action, SbVec3f & direction) const;
  void setPalette(const CvrCLUT * c);

  Cvr3DTexCube * volumecube;
  unsigned int voldatadims[3];
  SbVec3s subcubesize;
  SoVolumeReader * reader;

  uint32_t transferfuncid;
  const CvrCLUT * clut;
};

#endif // !COIN_CVRCUBEHANDLER_H
