#ifndef COIN_CVR3DPALETTETEXTURE_H
#define COIN_CVR3DPALETTETEXTURE_H

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

#include <Inventor/SbVec3s.h>
#include <VolumeViz/render/common/CvrPaletteTexture.h>

class CvrCLUT;


class Cvr3DPaletteTexture : public CvrPaletteTexture {
public:
  static void initClass(void);

  virtual SoType getTypeId(void) const;
  static SoType getClassTypeId(void);

  SbVec3s dimensions;
  virtual uint8_t * getIndex8Buffer(void) const;

  void blankUnused(const SbVec3s & texsize) const;

private:

  Cvr3DPaletteTexture(const SbVec3s & size);
  virtual ~Cvr3DPaletteTexture();

  static SoType classTypeId;

  friend class CvrVoxelChunk;
  friend class CvrTextureManager;
};

#endif // !COIN_CVR3DPALETTETEXTURE_H
