#ifndef COIN_SOVOLUMERENDER_H
#define COIN_SOVOLUMERENDER_H

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

#include <Inventor/nodes/SoNode.h>
#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/nodes/SoShape.h>
#include <Inventor/fields/SoSFEnum.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFVec3f.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoSFInt32.h>
#include <VolumeViz/C/basic.h>


class SIMVOLEON_DLL_API SoVolumeRender : public SoShape {
  typedef SoShape inherited;

  SO_NODE_HEADER(SoVolumeRender);

public:
  static void initClass(void);
  SoVolumeRender(void);

  enum Interpolation { NEAREST, LINEAR };
  enum Composition { MAX_INTENSITY, SUM_INTENSITY, ALPHA_BLENDING };
  enum NumSlicesControl { ALL, MANUAL, AUTOMATIC };

  enum AbortCode { CONTINUE, ABORT, SKIP };
  typedef AbortCode SoVolumeRenderAbortCB(int totalslices, int thisslice, 
                                          void * userdata);

  void setAbortCallback(SoVolumeRenderAbortCB * func, void * userdata = NULL);

  SoSFEnum interpolation;
  SoSFEnum composition;
  SoSFBool lighting;
  SoSFVec3f lightDirection;
  SoSFFloat lightIntensity;
  SoSFEnum numSlicesControl;
  SoSFInt32 numSlices;
  SoSFBool viewAlignedSlices;

protected:
  ~SoVolumeRender();

  virtual void GLRender(SoGLRenderAction * action);
  virtual void rayPick(SoRayPickAction * action);
  virtual void generatePrimitives(SoAction * action);
  virtual void computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center);

private:  
  friend class SoVolumeRenderP;
  class SoVolumeRenderP * pimpl;
};

#endif // !COIN_SOVOLUMERENDER_H
