#ifndef SIMVOLEON_CVRRESOURCEMANAGER_H
#define SIMVOLEON_CVRRESOURCEMANAGER_H

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

#include <Inventor/SbDict.h>
#include <Inventor/lists/SbList.h>
#include <Inventor/system/gl.h>

// *************************************************************************

class CvrResourceManager {
public:
  static CvrResourceManager * getInstance(uint32_t ctxid);

  typedef void ToBeDeletedCB(void * closure, uint32_t contextid);

  void set(const void * resourceholder, void * resource, ToBeDeletedCB * cb, void * cbclosure);
  SbBool get(const void * resourceholder, void *& resource) const;
  void remove(const void * resourceholder);

  void killTexture(const GLuint id);

private:
  CvrResourceManager(uint32_t ctxid);
  ~CvrResourceManager();

  uint32_t ctxid;
  static SbDict * managers;
  SbDict resourceholders;

  struct cb {
    const void * resourceholder;
    ToBeDeletedCB * func;
    void * closure;
  };

  SbList<struct cb> cblist;
  SbList<GLuint> dyingtextureids;
  void GLContextMadeCurrent(uint32_t contextid);
  static void GLContextMadeCurrentCB(void * closure, uint32_t contextid);
  static void GLContextDestructionCB(uint32_t ctxtid, void * userdata);
};

// *************************************************************************

#endif // !SIMVOLEON_CVRRESOURCEMANAGER_H
