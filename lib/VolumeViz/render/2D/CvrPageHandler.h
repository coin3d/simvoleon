#ifndef SIMVOLEON_CVRPAGEHANDLER_H
#define SIMVOLEON_CVRPAGEHANDLER_H

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
#include <VolumeViz/nodes/SoVolumeRender.h>

class Cvr2DTexPage;
class SoVolumeData;
class SoState;
class SoGLRenderAction;
class CvrCLUT;

// *************************************************************************

class CvrPageHandler {
public:
  CvrPageHandler(const SoGLRenderAction * action);
  ~CvrPageHandler();

  enum Composition { MAX_INTENSITY, SUM_INTENSITY, ALPHA_BLENDING };

  void render(const SoGLRenderAction * action, unsigned int numslices,
              // FIXME: composition should be passed on state
              // stack. 20040722 mortene.
              CvrPageHandler::Composition composition,
              // FIXME: abort-callback, and the numslices argument
              // above, should both be passed on the state
              // stack. 20040722 mortene.
              SoVolumeRender::SoVolumeRenderAbortCB * abortfunc,
              void * abortcbdata);

  unsigned int getCurrentAxis(const SoGLRenderAction * action) const;

  void releaseAllSlices(void);
  void releaseSlices(const unsigned int AXISIDX);

private:
  unsigned int getCurrentAxis(const SbVec3f & viewvec) const;
  void getViewVector(const SoGLRenderAction * action, SbVec3f & direction) const;
  Cvr2DTexPage * getSlice(const SoGLRenderAction * action,
                          const unsigned int AXISIDX, unsigned int sliceidx);

  // FIXME: should rather use an extension Coin cache, than using
  // comparison functions and storing the subpagesize
  // locally. 20040722 mortene.
  void comparePageSize(const SbVec3s & currsubpagesize);
  SbVec3s subpagesize;

  void setPalette(const CvrCLUT * c);

  Cvr2DTexPage ** slices[3];
  unsigned int voldatadims[3];

  uint32_t transferfuncid;
  const CvrCLUT * clut;
};

#endif // !SIMVOLEON_CVRPAGEHANDLER_H
