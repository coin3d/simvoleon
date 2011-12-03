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

#include <VolumeViz/render/common/CvrRGBATexture.h>

#include <assert.h>
#include <Inventor/SbName.h>

// *************************************************************************

// Don't set value explicitly to SoType::badType(), to avoid a bug in
// Sun CC v4.0. (Bitpattern 0x0000 equals SoType::badType()).
SoType CvrRGBATexture::classTypeId;

SoType CvrRGBATexture::getTypeId(void) const { return CvrRGBATexture::classTypeId; }
SoType CvrRGBATexture::getClassTypeId(void) { return CvrRGBATexture::classTypeId; }

// *************************************************************************

void
CvrRGBATexture::initClass(void)
{
  assert(CvrRGBATexture::classTypeId == SoType::badType());
  CvrRGBATexture::classTypeId =
    SoType::createType(CvrTextureObject::getClassTypeId(), "CvrRGBATexture");
}

CvrRGBATexture::CvrRGBATexture(void)
{
  assert(CvrRGBATexture::classTypeId != SoType::badType());
  this->rgbabuffer = NULL;
}

CvrRGBATexture::~CvrRGBATexture()
{
  if (this->rgbabuffer) delete[] this->rgbabuffer;
}

// *************************************************************************

// Returns pointer to RGBA buffer. Allocates memory for it if
// necessary.
uint32_t *
CvrRGBATexture::getRGBABuffer(void) const
{ 
  return this->rgbabuffer;
}

