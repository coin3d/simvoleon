#ifndef SIMVOLEON_SOORTHOSLICE_H
#define SIMVOLEON_SOORTHOSLICE_H

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

#include <Inventor/nodes/SoShape.h>
#include <Inventor/fields/SoSFUInt32.h>
#include <Inventor/fields/SoSFEnum.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFNode.h>

#include <VolumeViz/C/basic.h>

// *************************************************************************

class SIMVOLEON_DLL_API SoOrthoSlice : public SoShape {
  typedef SoShape inherited;

  SO_NODE_HEADER(SoOrthoSlice);

public:
  static void initClass(void);

  SoOrthoSlice(void);

  enum Axis { X = 0, Y, Z };
  enum Interpolation { NEAREST, LINEAR };
  enum AlphaUse { ALPHA_AS_IS, ALPHA_OPAQUE, ALPHA_BINARY };
  enum ClippingSide { FRONT, BACK };

  SoSFUInt32 sliceNumber;
  SoSFEnum axis;
  SoSFEnum interpolation;
  SoSFEnum alphaUse;
  SoSFEnum clippingSide;
  SoSFBool clipping;
  SoSFNode alternateRep;

  virtual SbBool affectsState(void) const;

protected:
  ~SoOrthoSlice();

  virtual void doAction(SoAction * action);
  virtual void GLRender(SoGLRenderAction * action);
  virtual void rayPick(SoRayPickAction * action);
  virtual void generatePrimitives(SoAction * action);
  virtual void computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center);
  virtual void write(SoWriteAction * action);

  virtual SbBool readInstance(SoInput * in, unsigned short flags);

private:
  friend class SoOrthoSliceP;
  class SoOrthoSliceP * pimpl;
};

#endif // !SIMVOLEON_SOORTHOSLICE_H
