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

#include <VolumeViz/render/common/CvrTextureObject.h>
#include <VolumeViz/render/common/CvrRGBATexture.h>
#include <VolumeViz/render/common/CvrPaletteTexture.h>
#include <VolumeViz/render/common/Cvr2DRGBATexture.h>
#include <VolumeViz/render/common/Cvr2DPaletteTexture.h>
#include <VolumeViz/render/common/Cvr3DRGBATexture.h>
#include <VolumeViz/render/common/Cvr3DPaletteTexture.h>
#include <Inventor/C/tidbits.h>
#include <Inventor/SbName.h>
#include <assert.h>

// Don't set value explicitly to SoType::badType(), to avoid a bug in
// Sun CC v4.0. (Bitpattern 0x0000 equals SoType::badType()).
SoType CvrTextureObject::classTypeId;

SoType CvrTextureObject::getTypeId(void) const { return CvrTextureObject::classTypeId; }
SoType CvrTextureObject::getClassTypeId(void) { return CvrTextureObject::classTypeId; }

void
CvrTextureObject::initClass(void)
{
  assert(CvrTextureObject::classTypeId == SoType::badType());
  CvrTextureObject::classTypeId = SoType::createType(SoType::badType(),
                                                     "CvrTextureObject");

  CvrRGBATexture::initClass();
  CvrPaletteTexture::initClass();
  Cvr2DRGBATexture::initClass();
  Cvr2DPaletteTexture::initClass();
  Cvr3DRGBATexture::initClass();
  Cvr3DPaletteTexture::initClass();

}

CvrTextureObject::CvrTextureObject()
{
  assert(CvrTextureObject::classTypeId != SoType::badType());
  this->opengltextureid = 0; // id=0 => texture has not been uploaded to texture mem. 
  this->iscompressed = FALSE;
  this->refcounter = 0;
}

CvrTextureObject::~CvrTextureObject()
{
}

GLuint 
CvrTextureObject::getOpenGLTextureId(void) const
{
  return this->opengltextureid;
}

void 
CvrTextureObject::setOpenGLTextureId(GLuint textureid)
{
  this->opengltextureid = textureid;
}

void
CvrTextureObject::setTextureCompressed(SbBool flag)
{
  this->iscompressed = flag;
}

SbBool
CvrTextureObject::textureCompressed() const
{
  return this->iscompressed;
}

void
CvrTextureObject::ref()
{
  this->refcounter++;
}

SbBool 
CvrTextureObject::unref()
{
  this->refcounter--;
  if (refcounter <= 0) // last unref?
    return FALSE;
  return TRUE;
}
