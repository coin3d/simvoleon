#ifndef COIN_SOOBLIQUESLICE_H
#define COIN_SOOBLIQUESLICE_H

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

// FIXME: class not yet implemented. 20040505 mortene.

#include <Inventor/nodes/SoShape.h>
#include <Inventor/fields/SoSFPlane.h>
#include <Inventor/fields/SoSFEnum.h>
#include <VolumeViz/C/basic.h>


class SIMVOLEON_DLL_API SoObliqueSlice : public SoShape {
  typedef SoShape inherited;

  SO_NODE_HEADER(SoObliqueSlice);

public:
  static void initClass(void);
  SoObliqueSlice(void);

  enum Interpolation { NEAREST, LINEAR };
  enum AlphaUse { ALPHA_AS_IS, ALPHA_OPAQUE, ALPHA_BINARY };

  SoSFPlane plane;
  SoSFEnum interpolation;
  SoSFEnum alphaUse;

protected:
  ~SoObliqueSlice();

  virtual void GLRender(SoGLRenderAction * action);
  virtual void rayPick(SoRayPickAction * action);
  virtual void generatePrimitives(SoAction * action);
  virtual void computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center);

private:
  friend class SoObliqueSliceP;
  class SoObliqueSliceP * pimpl;

  enum RenderingMethod { TEXTURE3D, TEXTURE2D, NOTIMPLEMENTED };

};

#endif // !COIN_SOOBLIQUESLICE_H
