#ifndef SIMVOLEON_CVRTEXTUREMANAGER_H
#define SIMVOLEON_CVRTEXTUREMANAGER_H

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

#include <Inventor/SbBox.h>
#include <Inventor/SbBox3s.h>
#include <Inventor/actions/SoGLRenderAction.h>

#include <VolumeViz/render/common/CvrTextureObject.h>
#include <VolumeViz/nodes/SoVolumeData.h>


class CvrTextureManager {

public:

  // 3D texture
  static const CvrTextureObject * getTextureObject(const SoGLRenderAction * action,
                                                   SoVolumeData * voldata,
                                                   SbVec3s texsize,
                                                   SbBox3s cutbox);

  static void finalizeTextureObject(const CvrTextureObject * textureobject);

private:
  static unsigned int totalNrOfTexels(void);
  static unsigned int totalTextureMemoryUsed(void);

  static CvrTextureObject * new3DTextureObject(const SoGLRenderAction * action,
                                               SoVolumeData * voldata,
                                               SbVec3s texsize,
                                               SbBox3s cutcube);

  static void transferTex3GL(const SoGLRenderAction * action,
                             CvrTextureObject * texobj,
                             const SbVec3s & texdims);

  static unsigned int totaltexturesize;
  static unsigned int totalnumberoftexels;
};

#endif // ! SIMVOLEON_CVRTEXTUREMANAGER_H
