/************************************************************************** \
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
 *  Systems in Motion, Postboks 1283, Pirsenteret, 7462 Trondheim, NORWAY.
 *  <URL:http://www.sim.no/>.
 *
\**************************************************************************/

#include <VolumeViz/render/common/CvrTextureObject.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include <assert.h>
#include <limits.h>

#include <Inventor/C/glue/gl.h>
#include <Inventor/C/tidbits.h>
#include <Inventor/SbBox2s.h>
#include <Inventor/SbName.h>
#include <Inventor/SbVec2s.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/elements/SoCacheElement.h>
#include <Inventor/errors/SoDebugError.h>

#include <VolumeViz/caches/CvrGLTextureCache.h>
#include <VolumeViz/elements/CvrCompressedTexturesElement.h>
#include <VolumeViz/elements/CvrGLInterpolationElement.h>
#include <VolumeViz/elements/CvrVoxelBlockElement.h>
#include <VolumeViz/elements/CvrLightingElement.h>
#include <VolumeViz/misc/CvrCLUT.h>
#include <VolumeViz/misc/CvrUtil.h>
#include <VolumeViz/misc/CvrVoxelChunk.h>
#include <VolumeViz/render/common/Cvr2DRGBATexture.h>
#include <VolumeViz/render/common/Cvr2DPaletteTexture.h>
#include <VolumeViz/render/common/Cvr3DRGBATexture.h>
#include <VolumeViz/render/common/Cvr3DPaletteTexture.h>
#include <VolumeViz/render/common/Cvr3DPaletteGradientTexture.h>

// *************************************************************************

// FIXME: suggestion found on comp.graphics.api.opengl on how to check
// how much texture VRAM is available:
//
// [...]
// Once you create texture(s), you can give it(them) a priority using
// glPrioritizeTexture
// (http://www.3dlabs.com/support/developer/GLmanpages/glprioritizetextures.htm)
//
// this should tell the ICD to put textures with the highest priority
// (and according to available texture memory on your hardware) into
// the actual video ram on you sweet geforce (or whatever card you
// happen to use:-).
//
// and then, you can see if that texture(s) is really resident (stored
// in vram) by calling glAreTexturesResident
// (http://www.3dlabs.com/support/developer/GLmanpages/glaretexturesresident.htm)
//
// So if you want to test how many textures you can cram into your
// vram, you can just keep on creating new textures and setting their
// priority to high (1) and for every new one you create see if it is
// really stored in vram.  Keep in mind that you need to use a given
// texture at least once before it can be "prioritized", put into
// vram.
//
// This is really really cool, because say you want to use 10 textures
// interchangeably. If you put them in vram then calls to
// glBindTexture will be ultra fricking fast, because the texture will
// no longer have to go through the AGP bus.
// [...]
//
// 20021130 mortene.
//
// Update 20021201 mortene: I think the technique above will be
// dangerous on UMA-machines (like the SGI O2) were tex mem is the
// same as other system mem. One would at least have to set a upper
// limit before running the test.


// For reference, here's some information from Thomas Roell of Xi
// Graphics on glPrioritizeTextures() from c.g.a.opengl:
//
// [...]
//
//   Texture priorities would be a nice thing, but only few OpenGL
//   implementations actually use them. There are a lot of reasons
//   that they ignore priorities. One is that the default priority is
//   set to 1.0, which is the highest priority. That means unless all
//   textures for all you applications running at the same time
//   explicitely use texture priorities, the one that uses them
//   (i.e. lower priorities) will be at a disadvantage. The other
//   problem is that typically textures are not the only objects that
//   live in HW accessable memory. There are display lists, color
//   tables, vertex array objects and so on. However there is no way
//   to prioritize them. Hence if you are using textures and display
//   lists at the same time, useng priorities might cause a lot of
//   texture cache trashing.
//
// [...]
//
// 20021201 mortene.

// *************************************************************************

// Don't set value explicitly to SoType::badType(), to avoid a bug in
// Sun CC v4.0. (Bitpattern 0x0000 equals SoType::badType()).
SoType CvrTextureObject::classTypeId;

SoType CvrTextureObject::getTypeId(void) const { return CvrTextureObject::classTypeId; }
SoType CvrTextureObject::getClassTypeId(void) { return CvrTextureObject::classTypeId; }

// *************************************************************************

SbDict * CvrTextureObject::instancedict = NULL;

// *************************************************************************

static SbBool
cvr_debug_textureuse(void)
{
  static int val = -1;
  if (val == -1) {
    const char * env = coin_getenv("CVR_DEBUG_TEXTUREUSE");
    val = env && (atoi(env) > 0);
  }
  return val > 0 ? TRUE : FALSE;
}

// *************************************************************************

void
CvrTextureObject::initClass(void)
{
  assert(CvrTextureObject::classTypeId == SoType::badType());
  CvrTextureObject::classTypeId = SoType::createType(SoType::badType(),
                                                     "CvrTextureObject");

  CvrRGBATexture::initClass();
  CvrPaletteTexture::initClass();
  Cvr2DRGBATexture::initClass();
  Cvr2DPaletteTexture::initClass();
  Cvr3DRGBATexture::initClass();
  Cvr3DPaletteTexture::initClass();
  Cvr3DPaletteGradientTexture::initClass();

  // FIXME: leak, never deallocated. 20040721 mortene.
  CvrTextureObject::instancedict = new SbDict;
}

CvrTextureObject::CvrTextureObject(void)
{
  assert(CvrTextureObject::classTypeId != SoType::badType());
  this->refcounter = 0;
}

CvrTextureObject::~CvrTextureObject()
{
  // Kill all existing CvrGLTextureCache instances (which will
  // indirectly deallocate GL textures):

  SbPList keys, values;
  this->glctxdict.makePList(keys, values);
  for (unsigned int i = 0; i < (unsigned int)values.getLength(); i++) {
    SbList<CvrGLTextureCache *> * l = (SbList<CvrGLTextureCache *> *)values[i];
    for (unsigned int j = 0; j < (unsigned int)l->getLength(); j++) { (*l)[j]->unref(); }
    delete l;
  }
  this->glctxdict.clear();


  // Take us out of the static list of all CvrTextureObject instances:

  const unsigned long key = this->hashKey();
  void * ptr;
  const SbBool ok = CvrTextureObject::instancedict->find(key, ptr);
  assert(ok);

  // Calculated hash key is not guaranteed to be unique, so a list is
  // stored in the hash, which we must do comparisons on the elements
  // in.
  SbList<CvrTextureObject *> * l = (SbList<CvrTextureObject *> *)ptr;
  const int idx = l->find(this);
  assert(idx != -1);
  l->removeFast(idx);

  if (l->getLength() == 0) {
    delete l;
    const SbBool ok = CvrTextureObject::instancedict->remove(key);
    assert(ok);
  }
}

// *************************************************************************

const SbVec3s &
CvrTextureObject::getDimensions(void) const
{
  return this->dimensions;
}

// *************************************************************************

SbList<CvrGLTextureCache *> *
CvrTextureObject::cacheListForGLContext(const uint32_t glctxid) const
{
  void * ptr;
  const SbBool found = this->glctxdict.find((unsigned long)glctxid, ptr);
  if (!found) { return NULL; }
  return (SbList<CvrGLTextureCache *> *)ptr;
}

SbBool
CvrTextureObject::findGLTexture(const SoGLRenderAction * action, GLuint & texid) const
{
  SbList<CvrGLTextureCache *> * cachelist =
    this->cacheListForGLContext(action->getCacheContext());
  if (cachelist == NULL) { return FALSE; }

  for (int i=0; i < cachelist->getLength(); i++) {
    CvrGLTextureCache * cache = (*cachelist)[i];

    if (cache->isDead()) {
      cache->unref();
      cachelist->remove(i);
      i--;
      continue;
    }

    if (cache->isValid(action->getState())) {
      texid = cache->getGLTextureId();
      return TRUE;
    }
  }
  return FALSE;
}

// *************************************************************************

// FIXME: the below should be separated out from this src
// file. (Ideally, the functions should be imported from a sub-module
// of the src/glue/gl*-stuff from Coin. But that refactoring job has
// not been done yet.) 20050628 mortene.

/*
  Convert an OpenGL enum error code to a textual representation.

  NOTE: from Coin's src/glue/gl.c. Should be removed when that module
  is separated out from Coin, and made available for inclusion into
  other CVS modules.
*/
static const char *
coin_glerror_string(GLenum errorcode)
{
  static const char INVALID_VALUE[] = "GL_INVALID_VALUE";
  static const char INVALID_ENUM[] = "GL_INVALID_ENUM";
  static const char INVALID_OPERATION[] = "GL_INVALID_OPERATION";
  static const char STACK_OVERFLOW[] = "GL_STACK_OVERFLOW";
  static const char STACK_UNDERFLOW[] = "GL_STACK_UNDERFLOW";
  static const char OUT_OF_MEMORY[] = "GL_OUT_OF_MEMORY";
  static const char unknown[] = "Unknown OpenGL error";

  switch (errorcode) {
  case GL_INVALID_VALUE:
    return INVALID_VALUE;
  case GL_INVALID_ENUM:
    return INVALID_ENUM;
  case GL_INVALID_OPERATION:
    return INVALID_OPERATION;
  case GL_STACK_OVERFLOW:
    return STACK_OVERFLOW;
  case GL_STACK_UNDERFLOW:
    return STACK_UNDERFLOW;
  case GL_OUT_OF_MEMORY:
    return OUT_OF_MEMORY;
  default:
    return unknown;
  }
  return NULL; /* avoid compiler warning */
}

/* Simple utility function for dumping the current set of error codes
   returned from glGetError(). Returns number of errors reported by
   OpenGL.

   NOTE: from Coin's src/glue/gl.c. Should be removed when that module
   is separated out from Coin, and made available for inclusion into
   other CVS modules.
*/
static unsigned int
coin_catch_gl_errors(cc_string * str)
{
  unsigned int errs = 0;
  GLenum glerr = glGetError();
  while (glerr != GL_NO_ERROR) {
    if (errs > 0) { cc_string_append_char(str, ' '); }
    cc_string_append_text(str, coin_glerror_string(glerr));
    errs++;
    glerr = glGetError();
  }
  return errs;
}

// *************************************************************************

GLuint
CvrTextureObject::getGLTexture(const SoGLRenderAction * action) const
{
  GLuint texid;
  if (this->findGLTexture(action, texid)) { return texid; }

  SoState * state = action->getState();

  // FIXME: why is this necessary? Investigate. 20040722 mortene.
  const SbBool storedinvalid = SoCacheElement::setInvalid(FALSE);

  // FIXME: is the value of the lighting flag picked up outside of the
  // cache tracking on purpose? Or is this a bug?  Investigate.
  // 20050628 mortene.
  const CvrLightingElement * lightelem = CvrLightingElement::getInstance(action->getState());
  assert(lightelem != NULL);
  const SbBool lighting = lightelem->useLighting(action->getState());

  // FIXME: in SoAsciiText, pederb uses this right after making a
  // cache -- what does this do?:
  //
  //    * SoCacheElement::addCacheDependency(state, cache); ???
  //
  // 20040722 mortene.

  CvrGLTextureCache * cache = new CvrGLTextureCache(state);
  cache->ref();

  // FIXME: check up exactly why this is done. 20040722 mortene.
  state->push();
  SoCacheElement::set(state, cache);

  const uint32_t glctxid = action->getCacheContext();
  const cc_glglue * glw = cc_glglue_instance(glctxid);

  // If texture is non-paletted, the "internalFormat" argument for
  // glTexImage[2|3]D() should usually be the number of components:
  GLenum internalFormat = 4;
  // FIXME: this is hackish -- how a paletted texture is handled
  // (through the palette extensions or through a fragment program)
  // should be stored in the CvrTextureObject's relevant
  // subclass. 20050701 mortene.
  if (this->isPaletted() &&
      CvrCLUT::usePaletteExtension(glw) &&
      !CvrCLUT::useFragmentProgramLookup(glw)) {
    internalFormat = GL_COLOR_INDEX8_EXT;
  }

  const SbVec3s texdims = this->getDimensions();

  // FIXME: glGenTextures() / glBindTexture() / glDeleteTextures() are
  // only supported in opengl >= 1.1, should have a fallback for 1.0
  // drivers, like we have in Coin, where we can use display lists
  // instead. 20040715 mortene.
  glGenTextures(1, &texid);
  assert(glGetError() == GL_NO_ERROR);

  const unsigned short nrtexdims = this->getNrOfTextureDimensions();
  const GLenum gltextypeenum = (nrtexdims == 2) ? GL_TEXTURE_2D : GL_TEXTURE_3D;

  glEnable(gltextypeenum);
  glBindTexture(gltextypeenum, texid);
  assert(glGetError() == GL_NO_ERROR);

#if CVR_DEBUG
  if (cvr_debug_textureuse()) {
    SoDebugError::postInfo("CvrTextureObject::getGLTexture",
                           "initial glBindTexture(%s, %u) in GL context %u",
                           (gltextypeenum == GL_TEXTURE_2D) ? "GL_TEXTURE_2D" : "GL_TEXTURE_3D",
                           texid,
                           action->getCacheContext());
  }
#endif // debug

  GLint wrapenum = GL_CLAMP;
  // FIXME: avoid using GL_CLAMP_TO_EDGE, since it may not be
  // available on all drivers. (Notably, it is missing from the
  // Microsoft OpenGL 1.1 software renderer, which is often used for
  // offscreen rendering on MSWin systems.)
  //
  // Disable this code, and fix any visual artifacts showing up. See
  // also description of bug #012 in SIMVoleon/BUGS.txt.
  //
  // 20040714 mortene.

  if (cc_glglue_has_texture_edge_clamp(glw) && (nrtexdims == 3)) {
    // We do this for now, to minimize the visible seams in the 3D
    // textures when interpolation is set to "LINEAR". See FIXME above
    // and bug #012.
    //
    // This work-around only active for 3D textures, as we can be
    // fairly certain we have at least OpenGL 1.2 (to which was added
    // both 3D textures and GL_CLAMP_TO_EDGE) if we get here. Besides,
    // the seams can be very ugly with 3D textures, but are usually
    // not easily visible with 2D textures, so there's less help in
    // this.
    wrapenum = GL_CLAMP_TO_EDGE;
  }

  glTexParameteri(gltextypeenum, GL_TEXTURE_WRAP_S, wrapenum);
  glTexParameteri(gltextypeenum, GL_TEXTURE_WRAP_T, wrapenum);
  if (nrtexdims == 3) {
    glTexParameteri(gltextypeenum, GL_TEXTURE_WRAP_R, wrapenum);
  }

  assert(glGetError() == GL_NO_ERROR);

  void * imgptr = NULL;
  if (this->isPaletted()) imgptr = ((CvrPaletteTexture *)this)->getIndex8Buffer();
  else imgptr = ((CvrRGBATexture *)this)->getRGBABuffer();


  GLenum gltextureformat = GL_COLOR_INDEX;
  if (this->isPaletted() && CvrCLUT::useFragmentProgramLookup(glw)) {
    if (lighting) {
      // Trick: use larger texture, to store the gradient, for access
      // from the fragment program(s). We're then using a 4-component
      // texture to store a luminance component plus 3 8-bits
      // vector-components for the gradient vector.
      gltextureformat = GL_RGBA;
    }
    else {
      gltextureformat = GL_LUMINANCE;
    }
  }

  // NOTE: Combining texture compression and GL_COLOR_INDEX doesn't
  // seem to work on NVIDIA cards (tested on GeForceFX 5600 &
  // GeForce2 MX) (20040316 handegar)
  //
  // FIXME: check if that is a general GL limitation. (I seem to
  // remember it is.) 20040716 mortene.
  //
  // FIXME: CvrCompressedTexturesElement should be FALSE if we're
  // using paletted textures, I believe, and if so, this should be an
  // assert, not an "if". 20041029 mortene.
  if (cc_glue_has_texture_compression(glw) && !this->isPaletted() &&
      // Important to check this last, as we want to avoid getting an
      // unnecessary cache dependency:
      CvrCompressedTexturesElement::get(state)) {
    assert(internalFormat == 4);
    internalFormat = GL_COMPRESSED_RGBA_ARB;
  }

  // Force this internal GL format if lighting is on, so we know which
  // order the components are in (which are <luminance, gradient0,
  // gradient1, gradient2>).
  //
  // FIXME: because of the store-gradient-in-texture trick, lighting
  // only works if we can do paletted textures -- is this checked
  // anywhere? Should ask kristian (or audit the code). 20050602 mortene.
  if (lighting) { internalFormat = GL_RGBA; }

  // By default we modulate textures with the material settings.
  if (!CvrUtil::dontModulateTextures()) {
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  }

  // Dump and empty the GL errors set before doing the GL calls below.
  {
    cc_string str;
    cc_string_construct(&str);
    const unsigned int nrerrors = coin_catch_gl_errors(&str);
    if (nrerrors != 0) {
      static SbBool first = TRUE;
      if (first) {
        SoDebugError::postWarning("CvrTextureObject::getGLTexture",
                                  "GL errors before glTexImage[2|3]D(): '%s' "
                                  "(displayed once, there may be repetitions)",
                                  cc_string_get_text(&str));
        first = FALSE;
      }
    }
    cc_string_clean(&str);
  }

  // FIXME: I guess we should really use proxy texture checking first,
  // before calling glTexImage[2|3]D() below. 20050628 mortene.





  if (nrtexdims == 2) {
    // Adding a border to get rid of seams between tiled textures
    //
    // See: http://www.opengl.org/resources/code/samples/sig99/advanced99/notes/node64.html
    //
    // Changes was also made to CvrVoxelChunk::buildSubPageX/Y/Z() as
    // well as increasing the texture size by 2 in
    // CvrTextureObject::create().
    //
    // 20090730 eigils
    int border = 1;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(gltextypeenum,
                 0,
                 internalFormat,
                 texdims[0]+2*border, texdims[1]+2*border,
                 border,
                 this->isPaletted() ? gltextureformat : GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 imgptr);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  }
  else {
    assert(nrtexdims == 3);
    cc_glglue_glTexImage3D(glw,
                           gltextypeenum,
                           0,
                           internalFormat,
                           texdims[0], texdims[1], texdims[2],
                           0,
                           this->isPaletted() ? gltextureformat : GL_RGBA,
                           GL_UNSIGNED_BYTE,
                           imgptr);
  }

  { // We've had a report of GL errors here, so dump lots of debug info.
    cc_string str;
    cc_string_construct(&str);
    const unsigned int nrerrors = coin_catch_gl_errors(&str);
    static SbBool first = TRUE;
    if ((nrerrors != 0) && first) {
      first = FALSE;
      SoDebugError::postWarning("CvrTextureObject::getGLTexture",
                                "GL errors: '%s' (displayed once, there may "
                                "be repetitions)",
                                cc_string_get_text(&str));
      if (nrtexdims == 2) {
        SoDebugError::postWarning("CvrTextureObject::getGLTexture",
                                  "error came from "
                                  "glTexImage2D(0x%x, 0, 0x%x, %d, %d, 0, 0x%x, GL_UNSIGNED_BYTE, %p)",
                                  gltextypeenum,
                                  internalFormat,
                                  texdims[0], texdims[1],
                                  this->isPaletted() ? gltextureformat : GL_RGBA,
                                  imgptr);
      }
      else {
        assert(nrtexdims == 3);
        SoDebugError::postWarning("CvrTextureObject::getGLTexture",
                                  "error came from "
                                  "glTexImage3D(0x%x, 0, 0x%x, %d, %d, %d, 0, 0x%x, GL_UNSIGNED_BYTE, %p)",
                                  gltextypeenum,
                                  internalFormat,
                                  texdims[0], texdims[1], texdims[2],
                                  this->isPaletted() ? gltextureformat : GL_RGBA,
                                  imgptr);
      }
    }
    cc_string_clean(&str);
  }

  cache->setGLTextureId(action, texid);

  SbList<CvrGLTextureCache *> * l = this->cacheListForGLContext(glctxid);
  if (l == NULL) {
    l = new SbList<CvrGLTextureCache *>;
    ((CvrTextureObject *)this)->glctxdict.enter((unsigned long)glctxid, l);
  }
  l->append(cache);

  state->pop();
  SoCacheElement::setInvalid(storedinvalid);

  return texid;
}

// *************************************************************************

uint32_t
CvrTextureObject::getRefCount(void) const
{
  return this->refcounter;
}

void
CvrTextureObject::ref(void) const
{
  ((CvrTextureObject *)this)->refcounter++;
}

void
CvrTextureObject::unref(void) const
{
  assert(this->refcounter > 0);
  ((CvrTextureObject *)this)->refcounter--;
  if (this->refcounter == 0) { delete this; }
}

// *************************************************************************

CvrTextureObject *
CvrTextureObject::findInstanceMatch(const SoType t,
                                    const struct CvrTextureObject::EqualityComparison & obj)
{
  const unsigned long key = CvrTextureObject::hashKey(obj);
  void * ptr;
  const SbBool ok = CvrTextureObject::instancedict->find(key, ptr);
  if (!ok) { return NULL; }

  // Calculated hash key is not guaranteed to be unique, so a list is
  // stored in the hash, which we must do comparisons on the elements
  // in.
  SbList<CvrTextureObject *> * l = (SbList<CvrTextureObject *> *)ptr;

  for (int i = 0; i < l->getLength(); i++) {
    CvrTextureObject * to = (*l)[i];
    if ((to->eqcmp == obj) && (to->getTypeId() == t)) { return to; }
  }

  return NULL;
}

/*! Returns an instance which embodies a chunk of voxels out of the
    current SoVolumeData on the state stack, as given by the \a
    cutcube argument.

    Automatically takes care of sharing if an instance was already
    made to the same specifications.
*/
const CvrTextureObject *
CvrTextureObject::create(const SoGLRenderAction * action,
                         const CvrCLUT * clut,
                         const SbBox3s & cutcube)
{
  const SbVec3s texsize(cutcube.getMax() - cutcube.getMin());
  const SbBox2s dummy; // constructor initializes it to an empty box
  return CvrTextureObject::create(action, clut, texsize, cutcube, dummy, UINT_MAX, INT_MAX);
}

const CvrTextureObject *
CvrTextureObject::create(const SoGLRenderAction * action,
                         const CvrCLUT * clut,
                         const SbVec2s & texsize,
                         const SbBox2s & cutslice,
                         const unsigned int axisidx,
                         const int pageidx)
{

  const SbVec3s tex(texsize[0], texsize[1], 1);

  const SbBox3s dummy; // constructor initializes it to an empty box

  return CvrTextureObject::create(action, clut, tex, dummy, cutslice, axisidx, pageidx);
}

// The common create function, used for both 2D and 3D cuts of the
// volume.
CvrTextureObject *
CvrTextureObject::create(const SoGLRenderAction * action,
                         const CvrCLUT * clut,
                         /* common: */ const SbVec3s & texsize,
                         /* 3D only: */ const SbBox3s & cutcube,
                         /* 2D only: */ const SbBox2s & cutslice, const unsigned int axisidx, const int pageidx)
{
  const CvrVoxelBlockElement * vbelem = CvrVoxelBlockElement::getInstance(action->getState());
  assert(vbelem != NULL);

  const SbBool paletted = CvrCLUT::usePaletteTextures(action);
  const SbBool is2d = (axisidx != UINT_MAX);

  const CvrLightingElement * lightelem = CvrLightingElement::getInstance(action->getState());
  assert(lightelem != NULL);
  const SbBool lighting = lightelem->useLighting(action->getState());

  SoType createtype;
  if (is2d && paletted) { createtype = Cvr2DPaletteTexture::getClassTypeId(); }
  else if (is2d) { createtype = Cvr2DRGBATexture::getClassTypeId(); }
  // FIXME: I believe this next may also depend on the presence of
  // support for fragment programs..? Investigate. 20050628 mortene.
  else if (paletted && lighting) { createtype = Cvr3DPaletteGradientTexture::getClassTypeId(); }
  else if (paletted) { createtype = Cvr3DPaletteTexture::getClassTypeId(); }
  else { createtype = Cvr3DRGBATexture::getClassTypeId(); }

  struct CvrTextureObject::EqualityComparison incoming;
  incoming.sovolumedata_id = vbelem->getNodeId();
  incoming.cutcube = cutcube; // For 3D tex
  incoming.cutslice = cutslice; // For 2D tex
  incoming.axisidx = axisidx; // For 2D tex
  incoming.pageidx = pageidx; // For 2D tex

  CvrTextureObject * obj =
    CvrTextureObject::findInstanceMatch(createtype, incoming);
  if (obj) { return obj; }

  const SbVec3s & voxdims = vbelem->getVoxelCubeDimensions();
  const void * dataptr = vbelem->getVoxels();

  // FIXME: improve buildSubPage() interface to fix this roundabout
  // way of calling it. 20021206 mortene.
  CvrVoxelChunk * input =
    new CvrVoxelChunk(voxdims, vbelem->getBytesPrVoxel(), dataptr);
  CvrVoxelChunk * cubechunk;
  if (is2d) { cubechunk = input->buildSubPage(axisidx, pageidx, cutslice); }
  else { cubechunk = input->buildSubCube(cutcube); }
  delete input;

  CvrTextureObject * newtexobj = (CvrTextureObject *)
    createtype.createInstance();

  // The actual dimensions of the GL texture must be values that are
  // power-of-two's:
  for (unsigned int i=0; i < 3; i++) {
    // SbMax(4, ...) to work around a crash bug in older NVidia
    // drivers when a lot of 1x- or 2x-dimensions textures are
    // allocated. 20090812 mortene.
    newtexobj->dimensions[i] =
      SbMax((uint32_t)4, coin_geq_power_of_two(texsize[i]));
  }

    if (is2d) {
      // code in many other spots adds the extra +2 to make room for
      // borders in 2D textures.
      //
      // FIXME: need to audit and figure out exactly why it can't /
      // shouldn't be done here. as it is now, the extra allocations
      // are spread out in a lot of places, which seems to me to be
      // somewhat unfortunate, as there are suddenly a lot of possible
      // points of failure for (often) hard to detect errors.
      //
      // 20090813 mortene.

  //    newtexobj->dimensions[0] += 2;
  //    newtexobj->dimensions[1] += 2;
      newtexobj->dimensions[2] = 1;
    }


  SbBool invisible = FALSE;
  cubechunk->transfer(action, clut, newtexobj, invisible);
  delete cubechunk;

  // If completely transparent, and not in palette mode, we need not
  // bother with a texture object for this slice/brick at all:
  if (invisible && !paletted) {
    // FIXME: we get grave mem-leaks by just returning here, I
    // believe. Audit code in this function. 20041029 mortene.
    return NULL;
  }

  // Must clear unused texture area to prevent artifacts due to
  // floating point inaccuracies when calculating texture coords.
  newtexobj->blankUnused(texsize);

  // We'll self-destruct when the SoVolumeData node is changed.
  //
  // FIXME: need to implement the self-destruction mechanism. Should
  // attach an SoNodeSensor to the SoVolumeData-node, have a
  // "about-to-be-destroyed" callback for the owners/auditors of
  // newtexobj, etc etc. 20040721 mortene.
  //
  // UPDATE: ..or is this already taken care of higher up in the
  // call-chain? I think it may be. Investigate. 20040722 mortene.
  newtexobj->eqcmp = incoming;

  const unsigned long key = newtexobj->hashKey();
  void * ptr;
  const SbBool ok = CvrTextureObject::instancedict->find(key, ptr);
  SbList<CvrTextureObject *> * l;
  if (ok) {
    l = (SbList<CvrTextureObject *> *)ptr;
  }
  else {
    l = new SbList<CvrTextureObject *>;
    const SbBool newentry = CvrTextureObject::instancedict->enter(key, l);
    assert(newentry);
  }
  l->append(newtexobj);

  return newtexobj;
}

// *************************************************************************

void
CvrTextureObject::activateTexture(const SoGLRenderAction * action) const
{
  const GLuint texid = this->getGLTexture(action);

  const unsigned short nrtexdims = this->getNrOfTextureDimensions();
  const GLenum gltextypeenum = (nrtexdims == 2) ? GL_TEXTURE_2D : GL_TEXTURE_3D;

  glEnable(gltextypeenum);
  glBindTexture(gltextypeenum, texid);

#if CVR_DEBUG
  if (cvr_debug_textureuse()) {
    SoDebugError::postInfo("CvrTextureObject::activeTexture",
                           "glBindTexture(%s, %u) in GL context %u",
                           (gltextypeenum == GL_TEXTURE_2D) ? "GL_TEXTURE_2D" : "GL_TEXTURE_3D",
                           texid, action->getCacheContext());
  }
#endif // debug

  const GLenum interp = CvrGLInterpolationElement::get(action->getState());
  glTexParameteri(gltextypeenum, GL_TEXTURE_MAG_FILTER, interp);
  glTexParameteri(gltextypeenum, GL_TEXTURE_MIN_FILTER, interp);

  assert(glGetError() == GL_NO_ERROR);

#if CVR_DEBUG && 0 // debug
  // FIXME: glAreTexturesResident() is OpenGL 1.1 only. 20021119 mortene.
  GLboolean residences[1];
  GLboolean resident = glAreTexturesResident(1, &texid, residences);
  if (!resident) {
    SoDebugError::postWarning("Cvr2DTexSubPage::activateTexture",
                              "texture %d not resident", texid);
    Cvr2DTexSubPage::detectedtextureswapping = TRUE;
  }

  // For reference, here's some information from Thomas Roell of Xi
  // Graphics on glAreTexturesResident() from c.g.a.opengl:
  //
  // [...]
  //
  //   With regards to glAreTexturesResident(), this is kind of
  //   tricky. This function returns which textures are currently
  //   resident is HW accessable memory (AGP, FB, TB). It does not
  //   return whether a set of textures could be made resident at a
  //   future point of time. A lot of OpenGL implementations (APPLE &
  //   XiGraphics for example) do cache a texture upon first use with
  //   3D primitive. Hence unless you had used a texture before it
  //   will not be resident. N.b that usually operations like
  //   glBindTexture, glTex*Image and so on will not make a texture
  //   resident for such caching implementations.
  //
  // [...]
  //
  // Additional information from Ian D Romanick (IBM engineer doing
  // Linux OpenGL work):
  //
  // [...]
  //
  //   AreTexturesResident is basically worthless, IMO.  All OpenGL
  //   rendering happens in a VERY high latency pipeline.  When an
  //   application calls AreTexturesResident, the textures may all be
  //   resident at that time.  However, there may already be
  //   primitives in the pipeline that will cause those textures to be
  //   removed from texturable memory before more primitives can be
  //   put in the pipe.
  //
  // [...]
  //
  // 20021201 mortene.

#endif // debug
}

// *************************************************************************

unsigned long
CvrTextureObject::hashKey(void) const
{
  return CvrTextureObject::hashKey(this->eqcmp);
}

unsigned long
CvrTextureObject::hashKey(const struct CvrTextureObject::EqualityComparison & obj)
{
  unsigned long key = obj.sovolumedata_id;

  if (obj.axisidx != UINT_MAX) { key += obj.axisidx; }
  if (obj.pageidx != INT_MAX) { key += obj.pageidx; }

  SbBox3s empty3;
  if (obj.cutcube.getMin() != empty3.getMin()) {
    short v[6];
    obj.cutcube.getBounds(v[0], v[1], v[2], v[3], v[4], v[5]);
    for (unsigned int i = 0; i < 6; i++) { key += v[i]; }
  }

  SbBox2s empty2;
  if (obj.cutslice.getMin() != empty2.getMin()) {
    short v[4];
    obj.cutslice.getBounds(v[0], v[1], v[2], v[3]);
    for (unsigned int i = 0; i < 4; i++) { key += v[i]; }
  }

  return key;
}

// *************************************************************************

int
CvrTextureObject::EqualityComparison::operator==(const CvrTextureObject::EqualityComparison & obj)
{
  return
    (this->sovolumedata_id == obj.sovolumedata_id) &&
    // Note: we compare SbBox3s corner points for the cutcube instead
    // of using the operator==() for SbBox3s, because the operator was
    // forgotten for export to the DLL interface up until and
    // including Coin version 2.3 (and SIM Voleon should be compatible
    // with anything from Coin 2.0 and upwards).
    (this->cutcube.getMin() == obj.cutcube.getMin()) &&
    (this->cutcube.getMax() == obj.cutcube.getMax()) &&
    (this->cutslice == obj.cutslice) &&
    (this->axisidx == obj.axisidx) &&
    (this->pageidx == obj.pageidx);

}

// *************************************************************************
