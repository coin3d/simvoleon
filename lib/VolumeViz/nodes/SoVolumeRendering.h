#ifndef COIN_SOVOLUMERENDERING_H
#define COIN_SOVOLUMERENDERING_H

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
#include <VolumeViz/C/basic.h>


class SIMVOLEON_DLL_API SoVolumeRendering : public SoNode {
  typedef SoNode inherited;

  SO_NODE_ABSTRACT_HEADER(SoVolumeRendering);

public:
  static void init(void);

  static void initClass(void);
  SoVolumeRendering();

  enum HW_Feature {
    HW_VOLUMEPRO, HW_3DTEXMAP, HW_TEXCOLORMAP, HW_TEXCOMPRESSION
  };

  enum HW_SupportStatus { NO, YES, UNKNOWN };

  HW_SupportStatus isSupported(HW_Feature feature);

protected:
  ~SoVolumeRendering();

private:
  friend class SoVolumeRenderingP;
  class SoVolumeRenderingP * pimpl;
};

#endif // !COIN_SOVOLUMERENDERING_H
