#ifndef SIMVOLEON_CVRRESOURCEMANAGER_H
#define SIMVOLEON_CVRRESOURCEMANAGER_H

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
};

// *************************************************************************

#endif // !SIMVOLEON_CVRRESOURCEMANAGER_H
