#ifndef SO_RAYCASTRENDER_H
#define SO_RAYCASTRENDER_H

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

// FIXME: Enable this #ifdef safety. (20100824 handegar)
/*
#ifndef HAVE_LIBCLVOL
#error "This node is not avalilable as SIMVoleon was not compiled with libCLVol support."
#else
*/

#include <Inventor/nodes/SoNode.h>
#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/nodes/SoShape.h>

#include <VolumeViz/C/basic.h>

class SIMVOLEON_DLL_API SoRaycastRender : public SoShape {
  typedef SoShape inherited;
  SO_NODE_HEADER(SoVolumeRender);

public:
  static void initClass(void);
  SoRaycastRender(void);

protected:
  ~SoRaycastRender();

  virtual void GLRender(SoGLRenderAction * action);
  virtual void rayPick(SoRayPickAction * action);
  virtual void generatePrimitives(SoAction * action);
  virtual void computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center);
  
private: 
  friend class SoRaycastRenderP;
  class SoRaycastRenderP * pimpl;
};

//#endif // !HAVE_LIBCLVOL
#endif // !SO_RAYCASTRENDER_H
