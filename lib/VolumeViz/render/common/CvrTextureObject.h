#ifndef COIN_CVRTEXTUREOBJECT_H
#define COIN_CVRTEXTUREOBJECT_H

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

#include <Inventor/SoType.h>
#include <Inventor/system/gl.h>

class SbVec3s;

// *************************************************************************

class CvrTextureObject {
public:
  static void initClass(void);

  virtual SoType getTypeId(void) const = 0;
  static SoType getClassTypeId(void);

  GLuint getOpenGLTextureId(void) const; 
  SbBool textureCompressed() const;
  void setTextureCompressed(SbBool flag);
  void setOpenGLTextureId(GLuint textureid);

  uint32_t getRefCount(void) const;
  void ref(void) const;
  void unref(void) const;

  virtual void blankUnused(const SbVec3s & texsize) const = 0;

protected:
  // Constructor and destructor is protected as only the texture
  // manager is allowed to create and remove TextureObjects.
  CvrTextureObject(void);
  virtual ~CvrTextureObject();

private:
  static SoType classTypeId;
  GLuint opengltextureid;
  SbBool iscompressed;
  uint32_t refcounter;

  // FIXME: This should be removed as soon as the 2D texture support is
  // implemented in the texture manager. (20040628 handegar)
  friend class Cvr2DTexPage;
};

#endif // !COIN_CVRTEXTUREOBJECT_H
