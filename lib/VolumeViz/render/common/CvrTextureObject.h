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

#include <GL/gl.h>

#include <Inventor/SbVec2s.h>
#include <Inventor/SoType.h>

class CvrTextureObject {
public:
  static void initClass(void);

  virtual SoType getTypeId(void) const = 0;
  static SoType getClassTypeId(void);

  GLuint getOpenGLTextureId(void) const; 
  SbBool textureCompressed() const;
  void setTextureCompressed(SbBool flag);
  void setOpenGLTextureId(GLuint textureid);

  void ref();
  SbBool unref();

private:

  // Constructor and destructor is private as only the texture manager
  // is allowed to create and remove TextureObjects.
  CvrTextureObject();
  virtual ~CvrTextureObject();

  static SoType classTypeId;
  GLuint opengltextureid;
  SbBool iscompressed;
  int refcounter;

  friend class CvrTextureManager;
  friend class CvrRGBATexture;
  friend class CvrPaletteTexture;
  // FIXME: This should be removed as soon as the 2D texture support is
  // implemented in the texture manager. (20040628 handegar)
  friend class Cvr2DTexPage;

};

#endif // !COIN_CVRTEXTUREOBJECT_H
