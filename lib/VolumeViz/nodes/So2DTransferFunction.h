#ifndef COIN_SO2DTRANSFERFUNCTION_H
#define COIN_SO2DTRANSFERFUNCTION_H

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

#include <Inventor/nodes/SoNode.h>
#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/fields/SoMFColorRGBA.h>
#include <Inventor/fields/SoMFFloat.h>
#include <Inventor/fields/SoMFVec2f.h>
#include <Inventor/fields/SoSFEnum.h>

#include <VolumeViz/C/basic.h>

class SIMVOLEON_DLL_API So2DTransferFunction : public SoNode {
  typedef So2DTransferFunction inherited;  
  SO_NODE_HEADER(So2DTransferFunction);

 public:
  static void initClass(void);
  So2DTransferFunction(void);

  enum PredefColorMap {
    GREY,
    GRAY = GREY,
    TEMPERATURE,
    PHYSICS,
    STANDARD,
    GLOW,
    BLUE_RED,
    SEISMIC
  };

  SoMFColorRGBA colors;
  SoMFFloat colorPoints;
  SoMFVec2f gradientPoints; // Not implemented yet.

  SoSFEnum predefColorMap;

 protected:
  ~So2DTransferFunction();
 
  virtual void doAction(SoAction * action);
  virtual void GLRender(SoGLRenderAction * action);
  virtual void callback(SoCallbackAction * action);
  virtual void pick(SoPickAction * action);

 private: 
  friend class So2DTransferFunctionP;
  class So2DTransferFunctionP * pimpl;
};

#endif // !COIN_SO2DTRANSFERFUNCTION_H
