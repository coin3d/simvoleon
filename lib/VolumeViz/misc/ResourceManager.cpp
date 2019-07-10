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
  const uintptr_t dictkey = (uintptr_t)resourceholder;
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
  const uintptr_t dictkey = (uintptr_t)resourceholder;
  return this->resourceholders.find(dictkey, resource);
}

void
CvrResourceManager::remove(const void * resourceholder)
{
  const uintptr_t dictkey = (uintptr_t)resourceholder;
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
