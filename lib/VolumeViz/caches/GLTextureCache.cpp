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
