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

#include <VolumeViz/misc/CvrResourceManager.h>

#include <Inventor/elements/SoGLCacheContextElement.h>
#include <Inventor/misc/SoContextHandler.h>

// *************************************************************************

SbDict * CvrResourceManager::managers = NULL;

// *************************************************************************

CvrResourceManager *
CvrResourceManager::getInstance(uint32_t ctxid)
{
  if (CvrResourceManager::managers == NULL) {
    // FIXME: this causes a static mem-leak. 20041111 mortene.
    CvrResourceManager::managers = new SbDict;
    SoContextHandler::addContextDestructionCallback(CvrResourceManager::GLContextDestructionCB, NULL);
  }

  const unsigned long key = (unsigned long)ctxid;
  void * value;
  const SbBool found = CvrResourceManager::managers->find(key, value);
  if (!found) {
    value = new CvrResourceManager(ctxid);
    const SbBool newentry = CvrResourceManager::managers->enter(key, value);
    assert(newentry);
  }
  return (CvrResourceManager *)value;
}

CvrResourceManager::CvrResourceManager(uint32_t ctxid)
{
  this->ctxid = ctxid;
}

CvrResourceManager::~CvrResourceManager()
{
}

// *************************************************************************

void
CvrResourceManager::set(const void * resourceholder, void * resource,
                        ToBeDeletedCB * cb, void * cbclosure)
{
  const unsigned long dictkey = (unsigned long)resourceholder;
  const SbBool newentry = this->resourceholders.enter(dictkey, resource);
  assert(newentry);

  if (cb) {
    struct cb s = { resourceholder, cb, cbclosure };
    this->cblist.append(s);
  }
}

SbBool
CvrResourceManager::get(const void * resourceholder, void *& resource) const
{
  const unsigned long dictkey = (unsigned long)resourceholder;
  return this->resourceholders.find(dictkey, resource);
}

void
CvrResourceManager::remove(const void * resourceholder)
{
  const unsigned long dictkey = (unsigned long)resourceholder;
  const SbBool ok = this->resourceholders.remove(dictkey);
  assert(ok);

  for (unsigned int i=0; i < (unsigned int)this->cblist.getLength(); i++) {
    if (this->cblist[i].resourceholder == resourceholder) {
      this->cblist.remove(i);
      break;
    }
  }
}

// *************************************************************************

void
CvrResourceManager::killTexture(const GLuint id)
{
  // If first item, schedule a callback to be invoked the next time
  // the context is made current.
  if (this->dyingtextureids.getLength() == 0) {
    SoGLCacheContextElement::scheduleDeleteCallback(this->ctxid,
                                                    CvrResourceManager::GLContextMadeCurrentCB,
                                                    this);
  }

  this->dyingtextureids.append(id);
}

// *************************************************************************

void
CvrResourceManager::GLContextMadeCurrent(uint32_t contextid)
{
  assert(contextid == this->ctxid);

  const unsigned int len = (unsigned int)this->dyingtextureids.getLength();
  for (unsigned int i=0; i < len; i++) {
    const GLuint id = this->dyingtextureids[i];
    glDeleteTextures(1, &id);
  }
  this->dyingtextureids.truncate(0);
}

void
CvrResourceManager::GLContextMadeCurrentCB(void * closure, uint32_t contextid)
{
  ((CvrResourceManager *)closure)->GLContextMadeCurrent(contextid);
}

// *************************************************************************

void
CvrResourceManager::GLContextDestructionCB(uint32_t contextid, void * userdata)
{
  CvrResourceManager * rm = CvrResourceManager::getInstance(contextid);

  while (rm->cblist.getLength()) {
    int len = rm->cblist.getLength();
    int idx = len - 1;
    struct cb * cbstruct = &(rm->cblist[idx]);
    cbstruct->func(cbstruct->closure, contextid);
    if (len == rm->cblist.getLength()) { rm->remove(cbstruct->resourceholder); }
  }

  // Clean out any resources recently added.
  rm->GLContextMadeCurrent(contextid);

  const unsigned long key = (unsigned long)contextid;
  const SbBool ok = CvrResourceManager::managers->remove(key);
  assert(ok);
  delete rm;
}

// *************************************************************************
