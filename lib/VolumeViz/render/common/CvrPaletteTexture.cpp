/**************************************************************************\
 * Copyright (c) Kongsberg Oil & Gas Technologies AS
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
