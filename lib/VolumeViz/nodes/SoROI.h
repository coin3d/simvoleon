#ifndef COIN_SOROI_H
#define COIN_SOROI_H

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

// FIXME: class not yet properly implemented. 20040505 mortene.


#include <VolumeViz/nodes/SoVolumeRendering.h>
#include <Inventor/fields/SoSFBox3s.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFEnum.h>


class SIMVOLEON_DLL_API SoROI : public SoVolumeRendering {
  typedef SoVolumeRendering inherited;

  SO_NODE_HEADER(SoROI);

public:
  SoROI(void);
  static void initClass(void);

  enum Flags {
    ENABLE_X0 = 0x1,
    ENABLE_Y0 = 0x2,
    ENABLE_Z0 = 0x4,
    INVERT_0 = 0x8,
    ENABLE_X1 = 0x10,
    ENABLE_Y1 = 0x20,
    ENABLE_Z1 = 0x40,
    INVERT_1 = 0x80,
    ENABLE_X2 = 0x100,
    ENABLE_Y2 = 0x200,
    ENABLE_Z2 = 0x400,
    INVERT_2 = 0x800,
    OR_SELECT = 0x1000,
    INVERT_OUTPUT = 0x2000,
    SUB_VOLUME = ENABLE_X0 | ENABLE_Y0 | ENABLE_Z0,
    EXCLUSION_BOX = SUB_VOLUME | INVERT_OUTPUT,
    CROSS = ENABLE_X0 | ENABLE_Y0 | ENABLE_Y1 | ENABLE_Z1 | ENABLE_X2 | ENABLE_Z2 | OR_SELECT,
    CROSS_INVERT = CROSS | INVERT_OUTPUT,
    FENCE = ENABLE_X0 | ENABLE_Y1 | ENABLE_Z2 | OR_SELECT,
    FENCE_INVERT = FENCE | INVERT_OUTPUT
  };

  SoSFBox3s box;
  SoSFEnum flags;
  SoSFBox3s subVolume;
  SoSFBool relative;

protected:
  ~SoROI();

  virtual void GLRender(SoGLRenderAction * action);

  // FIXME: Implement these functions... torbjorv 07312002
  virtual void doAction(SoAction * action);
  virtual void callback(SoCallbackAction * action);
  virtual void getBoundingBox(SoGetBoundingBoxAction * action);
  virtual void pick(SoPickAction * action);

private:
  friend class SoROIP;
  class SoROIP * pimpl;
};

#endif // !COIN_SOROI_H
