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

#include <VolumeViz/caches/CvrGLTextureCache.h>

#include <limits.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <VolumeViz/misc/CvrResourceManager.h>

// *************************************************************************

CvrGLTextureCache::CvrGLTextureCache(SoState * state)
  : SoCache(state)
{
  this->texid = 0;
  this->glctxid = UINT_MAX;
  this->dead = FALSE;
}

CvrGLTextureCache::~CvrGLTextureCache()
{
  if (!this->isDead() && (this->texid != 0)) {
    CvrResourceManager * rm = CvrResourceManager::getInstance(this->glctxid);
    rm->killTexture(this->texid);
    rm->remove(this);
  }
}

// *************************************************************************

void
CvrGLTextureCache::texDestructionCB(void * closure, uint32_t ctxid)
{
  CvrGLTextureCache * thisp = (CvrGLTextureCache *)closure;

  glDeleteTextures(1, &thisp->texid);

  thisp->dead = TRUE;
  thisp->texid = 0;
}

// *************************************************************************

void
CvrGLTextureCache::setGLTextureId(const SoGLRenderAction * action, GLuint id)
{
  assert((this->texid == 0) && "can not reset texid value");
  assert(!this->dead);

  this->texid = id;
  this->glctxid = action->getCacheContext();

  CvrResourceManager * rm = CvrResourceManager::getInstance(this->glctxid);
  rm->set(this, NULL, CvrGLTextureCache::texDestructionCB, this);
}

GLuint
CvrGLTextureCache::getGLTextureId(void) const
{
  assert(!this->dead);
  return this->texid;
}

// *************************************************************************

/*! Returns \c TRUE if the texture has been deallocated.

    This would usually be due to GL context destruction. It could also
    happen on other whims of internal resource manager, e.g. it could
    be thrown out due to texture aging.

    Client code which owns this cache should regularly check the
    isDead() flag and unref() the cache if detected to be \c TRUE.
*/
SbBool
CvrGLTextureCache::isDead(void) const
{
  return this->dead;
}

// *************************************************************************
