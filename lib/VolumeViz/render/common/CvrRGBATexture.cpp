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

#include <VolumeViz/render/common/CvrRGBATexture.h>
#include <Inventor/SbName.h>
#include <assert.h>

// Don't set value explicitly to SoType::badType(), to avoid a bug in
// Sun CC v4.0. (Bitpattern 0x0000 equals SoType::badType()).
SoType CvrRGBATexture::classTypeId;

SoType CvrRGBATexture::getTypeId(void) const { return CvrRGBATexture::classTypeId; }
SoType CvrRGBATexture::getClassTypeId(void) { return CvrRGBATexture::classTypeId; }

void
CvrRGBATexture::initClass(void)
{
  assert(CvrRGBATexture::classTypeId == SoType::badType());
  CvrRGBATexture::classTypeId =
    SoType::createType(CvrTextureObject::getClassTypeId(), "CvrRGBATexture");
}

CvrRGBATexture::CvrRGBATexture()
{
  assert(CvrRGBATexture::classTypeId != SoType::badType());
  this->rgbabuffer = NULL;
}

CvrRGBATexture::~CvrRGBATexture()
{
  if (this->rgbabuffer) delete[] this->rgbabuffer;
}

// Returns pointer to RGBA buffer. Allocates memory for it if
// necessary.
uint32_t *
CvrRGBATexture::getRGBABuffer(void) const
{ 
  return this->rgbabuffer;
}

