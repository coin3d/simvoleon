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

#include <VolumeViz/render/common/CvrPaletteTexture.h>

#include <assert.h>
#include <Inventor/SbName.h>
#include <VolumeViz/misc/CvrCLUT.h>

// *************************************************************************

// Don't set value explicitly to SoType::badType(), to avoid a bug in
// Sun CC v4.0. (Bitpattern 0x0000 equals SoType::badType()).
SoType CvrPaletteTexture::classTypeId;

SoType CvrPaletteTexture::getTypeId(void) const { return CvrPaletteTexture::classTypeId; }
SoType CvrPaletteTexture::getClassTypeId(void) { return CvrPaletteTexture::classTypeId; }

// *************************************************************************

void
CvrPaletteTexture::initClass(void)
{
  assert(CvrPaletteTexture::classTypeId == SoType::badType());
  CvrPaletteTexture::classTypeId =
    SoType::createType(CvrTextureObject::getClassTypeId(), "CvrPaletteTexture");
}

CvrPaletteTexture::CvrPaletteTexture(void)
{
  assert(CvrPaletteTexture::classTypeId != SoType::badType());
  this->indexbuffer = NULL;
  this->clut = NULL;
}

CvrPaletteTexture::~CvrPaletteTexture()
{
  if (this->clut) this->clut->unref();
  if (this->indexbuffer) delete [] this->indexbuffer;
}

// *************************************************************************

// Returns pointer to buffer with 8-bit indices.
uint8_t *
CvrPaletteTexture::getIndex8Buffer(void) const
{
  return this->indexbuffer;
}

// *************************************************************************

void
CvrPaletteTexture::setCLUT(const CvrCLUT * table)
{
  if (this->clut) this->clut->unref();
  this->clut = table;
  this->clut->ref();
}

const CvrCLUT *
CvrPaletteTexture::getCLUT(void) const
{
  return this->clut;
}

// *************************************************************************
