#ifndef SIMVOLEON_CVRTEXTUREOBJECT_H
#define SIMVOLEON_CVRTEXTUREOBJECT_H

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

#include <Inventor/SoType.h>
#include <Inventor/SbVec3s.h>
#include <Inventor/SbBox2s.h>
#include <Inventor/SbBox3s.h>
#include <Inventor/system/gl.h>
#include <Inventor/lists/SbList.h>
#include <VolumeViz/misc/CvrCLUT.h>

class SoGLRenderAction;
class SbVec2s;
class CvrGLTextureCache;

// *************************************************************************

class CvrTextureObject {
public:
  static const CvrTextureObject * create(const SoGLRenderAction * action,
                                         const CvrCLUT * clut,
                                         const SbBox3s & cutcube);


  static const CvrTextureObject * create(const SoGLRenderAction * action,
                                         const CvrCLUT * clut,
                                         const SbVec2s & texsize,
                                         const SbBox2s & cutslice,
                                         const unsigned int axisidx,
                                         const int pageidx);

  static void initClass(void);

  virtual SoType getTypeId(void) const = 0;
  static SoType getClassTypeId(void);

  uint32_t getRefCount(void) const;
  void ref(void) const;
  void unref(void) const;

  const SbVec3s & getDimensions(void) const;

  void activateTexture(const SoGLRenderAction * action) const;

  virtual SbBool isPaletted(void) const = 0;
  virtual void blankUnused(const SbVec3s & texsize) const = 0;
  virtual unsigned short getNrOfTextureDimensions(void) const = 0;

protected:
  // Constructor and destructor is protected as instances should
  // always be made through the create() function.
  CvrTextureObject(void);
  virtual ~CvrTextureObject();

private:
  SbBool findGLTexture(const SoGLRenderAction * action, GLuint & texid) const;
  static CvrTextureObject * create(const SoGLRenderAction * action,
                                   const CvrCLUT * clut,
                                   const SbVec3s & texsize,
                                   const SbBox3s & cutcube,
                                   const SbBox2s & cutslice,
                                   const unsigned int axisidx,
                                   const int pageidx);

  GLuint getGLTexture(const SoGLRenderAction * action) const;

  static SoType classTypeId;
  SbVec3s dimensions;
  uint32_t refcounter;
  static SbDict * instancedict;
  SbDict glctxdict;

  SbList<CvrGLTextureCache *> * cacheListForGLContext(const uint32_t glctxid) const;

  struct EqualityComparison {
    uint32_t sovolumedata_id;
    // FIXME: messy, next data should be part of subclasses. 20040721 mortene.
    // for 3D cuts:
    SbBox3s cutcube;
    // these are for 2D cuts:
    SbBox2s cutslice;
    unsigned int axisidx;
    int pageidx;

    int operator==(const struct EqualityComparison & cmp);
  } eqcmp;

  static CvrTextureObject * findInstanceMatch(const SoType t,
                                              const struct CvrTextureObject::EqualityComparison & cmp);
  unsigned long hashKey(void) const;
  static unsigned long hashKey(const struct CvrTextureObject::EqualityComparison & cmp);
};

#endif // !SIMVOLEON_CVRTEXTUREOBJECT_H
