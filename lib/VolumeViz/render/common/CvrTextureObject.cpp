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

#include <VolumeViz/render/common/CvrTextureObject.h>

#include <assert.h>
#include <limits.h>

#include <Inventor/C/glue/gl.h>
#include <Inventor/C/tidbits.h>
#include <Inventor/SbName.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/SbVec2s.h>
#include <Inventor/SbBox2s.h>

#include <VolumeViz/elements/CvrCompressedTexturesElement.h>
#include <VolumeViz/elements/CvrGLInterpolationElement.h>
#include <VolumeViz/elements/CvrVoxelBlockElement.h>
#include <VolumeViz/misc/CvrUtil.h>
#include <VolumeViz/misc/CvrVoxelChunk.h>
#include <VolumeViz/render/common/Cvr2DRGBATexture.h>
#include <VolumeViz/render/common/Cvr2DPaletteTexture.h>
#include <VolumeViz/render/common/Cvr3DRGBATexture.h>
#include <VolumeViz/render/common/Cvr3DPaletteTexture.h>
#include <VolumeViz/render/common/resourcehandler.h>

// *************************************************************************

// Don't set value explicitly to SoType::badType(), to avoid a bug in
// Sun CC v4.0. (Bitpattern 0x0000 equals SoType::badType()).
SoType CvrTextureObject::classTypeId;

SoType CvrTextureObject::getTypeId(void) const { return CvrTextureObject::classTypeId; }
SoType CvrTextureObject::getClassTypeId(void) { return CvrTextureObject::classTypeId; }

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
}

CvrTextureObject::CvrTextureObject(const SbVec3s & size)
{
  assert(CvrTextureObject::classTypeId != SoType::badType());
  this->refcounter = 0;

  assert(coin_is_power_of_two(size[0]));
  assert(coin_is_power_of_two(size[1]));
  assert(coin_is_power_of_two(size[2]));
  this->dimensions = size;
}

CvrTextureObject::~CvrTextureObject()
{
}

// *************************************************************************

const SbVec3s &
CvrTextureObject::getDimensions(void) const
{
  return this->dimensions;
}

// *************************************************************************

#if 0 // FIXME: reenable this code, but integrated in this
      // class. 20040716 mortene.
void
CvrTextureManager::finalizeTextureObject(const CvrTextureObject * texobject)
{
  if (texobject->getRefCount() == 1) { // about to be destructed
    GLuint tmp[1];
    tmp[0] = texobject->getOpenGLTextureId();
    glDeleteTextures(1, tmp);

    void * ptr;
    get_texturedict()->find((unsigned long)texobject, ptr);
    assert(ptr && "Trying to remove a CvrTextureObject which has not been registered!");
    const SbBool ok = get_texturedict()->remove((unsigned long)texobject);
    assert(ok);
    delete ((textureobj *)ptr);
  }

  texobject->unref();
}
#endif

// FIXME: a GL texture for a CvrTextureObject is not only dependent on
// the context, but also various things that are on the state stack
// ("texture compression wanted" flag, for instance).
//
// The best way to handle this is properly by using the Coin cache
// mechanism? Check how pederb did the font caching, which should be a
// simple example to follow.
//
// 20040716 mortene.
SbBool
CvrTextureObject::findGLTexture(const SoGLRenderAction * action, GLuint & texid) const
{
  uint32_t glctxid = action->getCacheContext();
  void * resource;
  const SbBool found = cvr_rc_find_resource(glctxid, (void *)this, &resource);

  if (!found) { return FALSE; }
  // FIXME: ugly cast. 20040716 mortene.
  texid = (GLuint)resource;
  return TRUE;
}

GLuint 
CvrTextureObject::getGLTexture(const SoGLRenderAction * action) const
{
  GLuint texid;
  if (this->findGLTexture(action, texid)) { return texid; }

  const uint32_t glctxid = action->getCacheContext();
  const cc_glglue * glw = cc_glglue_instance(glctxid);

  // Default values is for RGBA-texture:
  int colorformat = 4;
  unsigned int bitspertexel = 32;  // 8 bits each R, G, B & A
  if (this->isPaletted()) {
    colorformat = GL_COLOR_INDEX8_EXT;
    bitspertexel = 8;
  }

  const SbVec3s texdims = this->getDimensions();

  // FIXME: resource usage counting not implemented yet. 20040716 mortene.
#if 0 // tmp disabled
  const unsigned int nrtexels = texdims[0] * texdims[1] * texdims[2];
  const unsigned int texmem = (unsigned int)(nrtexels * bitspertexel / 8);
#endif

  // FIXME: glGenTextures() / glBindTexture() / glDeleteTextures() are
  // only supported in opengl >= 1.1, should have a fallback for 1.0
  // drivers, like we have in Coin, where we can use display lists
  // instead. 20040715 mortene.
  glGenTextures(1, &texid);
  assert(glGetError() == GL_NO_ERROR);

  const GLenum gltextypeenum = this->getGLTextureEnum();

  glEnable(gltextypeenum);
  glBindTexture(gltextypeenum, texid);
  assert(glGetError() == GL_NO_ERROR);

  GLint wrapenum = GL_CLAMP;
  // FIXME: avoid using GL_CLAMP_TO_EDGE, since it may not be
  // available on all drivers. (Notably, it is missing from the
  // Microsoft OpenGL 1.1 software renderer, which is often used for
  // offscreen rendering on MSWin systems.)
  //
  // Let this code be disabled for a while, and fix any visual
  // artifacts showing up -- preferably *without* re-enabling the
  // use of GL_CLAMP_TO_EDGE again. Eventually, we should simply
  // just remove the below disabled code, if we find that we can
  // actually do without it.
  //
  // 20040714 mortene.
#if 0
  if (cc_glglue_has_texture_edge_clamp(glw)) { wrapenum = GL_CLAMP_TO_EDGE; }
#endif
  glTexParameteri(gltextypeenum, GL_TEXTURE_WRAP_S, wrapenum);
  glTexParameteri(gltextypeenum, GL_TEXTURE_WRAP_T, wrapenum);
  if (gltextypeenum == GL_TEXTURE_3D) {
    glTexParameteri(gltextypeenum, GL_TEXTURE_WRAP_R, wrapenum);
  }

  assert(glGetError() == GL_NO_ERROR);

  void * imgptr = NULL;
  if (this->isPaletted()) imgptr = ((CvrPaletteTexture *)this)->getIndex8Buffer();
  else imgptr = ((CvrRGBATexture *)this)->getRGBABuffer();


  int palettetype = GL_COLOR_INDEX;
#ifdef HAVE_ARB_FRAGMENT_PROGRAM
  if (cc_glglue_has_arb_fragment_program(glw)) { palettetype = GL_LUMINANCE; }
#endif // HAVE_ARB_FRAGMENT_PROGRAM

  SoState * state = action->getState();

  // NOTE: Combining texture compression and GL_COLOR_INDEX doesn't
  // seem to work on NVIDIA cards (tested on GeForceFX 5600 &
  // GeForce2 MX) (20040316 handegar)
  //
  // FIXME: check if that is a general GL limitation. (I seem to
  // remember it is.) 20040716 mortene.
  //
  // FIXME: compression setting is a property of the GL texture which
  // should be part of the comparison check when we figure out whether
  // to make a new texture, or to reuse an exisiting one. Should
  // probably best take care of this by using the Coin caching
  // mechanism. 20040716 mortene.
  if (cc_glue_has_texture_compression(glw) &&
      (palettetype != GL_COLOR_INDEX) &&
      // Important to check this last, as we want to avoid getting an
      // unnecessary cache dependency:
      CvrCompressedTexturesElement::get(state)) {
    if (colorformat == 4) colorformat = GL_COMPRESSED_RGBA_ARB;
    else colorformat = GL_COMPRESSED_INTENSITY_ARB;
  }

  // By default we modulate textures with the material settings.
  if (!CvrUtil::dontModulateTextures()) {
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  }

  if (gltextypeenum == GL_TEXTURE_2D) {
    glTexImage2D(gltextypeenum,
                 0,
                 colorformat,
                 texdims[0], texdims[1],
                 0,
                 this->isPaletted() ? palettetype : GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 imgptr);
  }
  else {
    cc_glglue_glTexImage3D(glw,
                           gltextypeenum,
                           0,
                           colorformat,
                           texdims[0], texdims[1], texdims[2],
                           0,
                           this->isPaletted() ? palettetype : GL_RGBA,
                           GL_UNSIGNED_BYTE,
                           imgptr);
  }

  assert(glGetError() == GL_NO_ERROR);

  cvr_rc_bind_resource(glctxid, (void *)this, (void *)texid);

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

// FIXME: remove this? Write comparison functions into
// CvrTextureObject? 20040716 mortene.

struct textureobj {
  uint32_t voldataid; // This will change if volume-data is modified
                      // in any way.

  CvrTextureObject * object;

  // For 3D textures:
  SbBox3s cutcube;
  // For 2D textures:
  SbBox2s cutslice;
  unsigned int axisidx;
  int pageidx;
};

// *************************************************************************

static SbDict *
get_texturedict(void)
{
  static SbDict * texturedict = NULL;

  if (texturedict == NULL) {
    texturedict = new SbDict;
    // FIXME: leak. 20040715 mortene.
  }
  return texturedict;
}

// *************************************************************************

static textureobj *
find_textureobject(const uint32_t voldataid,
                   const SbBox3s cutcube,
                   const SbBox2s & cutslice,
                   const unsigned int axisidx,
                   const int pageidx)
{
  SbPList keylist;
  SbPList valuelist;

  // FIXME: traversing as a list is too slow with 2D textures!
  // 20040720 mortene.
  get_texturedict()->makePList(keylist, valuelist);

  for (int i=0;i<valuelist.getLength();++i) {
    void * ptr = valuelist[i];
    textureobj * obj = (textureobj *) ptr;
    // FIXME: this cmp operation is probably not strict
    // enough. Audit. 20040716 mortene.
    if ((obj->voldataid == voldataid) &&
        // Note: we compare SbBox3s corner points for the cutcube
        // instead of using the operator==() for SbBox3s, because the
        // operator was forgotten for export to the DLL interface up
        // until and including Coin version 2.3 (and SIM Voleon should
        // be compatible with anything from Coin 2.0 and upwards).
        (obj->cutcube.getMin() == cutcube.getMin()) &&
        (obj->cutcube.getMax() == cutcube.getMax()) &&
        (obj->cutslice == cutslice) &&
        (obj->axisidx == axisidx) &&
        (obj->pageidx == pageidx)) {
      return obj;
    }
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
                         const SbVec3s & texsize,
                         const SbBox3s & cutcube)
{
  const SbBox2s dummy; // constructor initializes it to an empty box
  return CvrTextureObject::newTextureObject(action, texsize, cutcube, dummy, UINT_MAX, INT_MAX);
}

const CvrTextureObject *
CvrTextureObject::create(const SoGLRenderAction * action,
                         const SbVec2s & texsize,
                         const SbBox2s & cutslice,
                         const unsigned int axisidx,
                         const int pageidx)
{
  const SbVec3s tex(texsize[0], texsize[1], 1);
  const SbBox3s dummy; // constructor initializes it to an empty box

  return CvrTextureObject::newTextureObject(action, tex, dummy, cutslice, axisidx, pageidx);
}

CvrTextureObject *
CvrTextureObject::newTextureObject(const SoGLRenderAction * action,
                                   const SbVec3s & texsize,
                                   const SbBox3s & cutcube,
                                   const SbBox2s & cutslice,
                                   const unsigned int axisidx,
                                   const int pageidx)
{
  const CvrVoxelBlockElement * vbelem = CvrVoxelBlockElement::getInstance(action->getState());
  assert(vbelem != NULL);

  textureobj * obj = find_textureobject(vbelem->getNodeId(),
                                        /* for 3d: */ cutcube,
                                        /* for 2d: */ cutslice, axisidx, pageidx);
  if (obj != NULL) { return obj->object; }

  const SbVec3s & vddims = vbelem->getVoxelCubeDimensions();
  const void * dataptr = vbelem->getVoxels();

  const SbBool is2d = (axisidx != UINT_MAX);

  // FIXME: improve buildSubPage() interface to fix this roundabout
  // way of calling it. 20021206 mortene.
  CvrVoxelChunk * input =
    new CvrVoxelChunk(vddims, vbelem->getBytesPrVoxel(), dataptr);
  CvrVoxelChunk * cubechunk;
  if (is2d) { cubechunk = input->buildSubPage(axisidx, pageidx, cutslice); }
  else { cubechunk = input->buildSubCube(cutcube); }
  delete input;

  SbBool invisible = FALSE;
  CvrTextureObject * newtexobj;
  if (is2d) { newtexobj = cubechunk->transfer2D(action, invisible); }
  else { newtexobj = cubechunk->transfer3D(action, invisible); }

  // If completely transparent
  if (invisible) { return NULL; }

  // Must clear unused texture area to prevent artifacts due to
  // floating point inaccuracies when calculating texture coords.
  newtexobj->blankUnused(texsize);

  obj = new textureobj;
  obj->object = newtexobj;
  obj->voldataid = vbelem->getNodeId();
  obj->cutcube = cutcube;
  obj->cutslice = cutslice;
  obj->axisidx = axisidx;
  obj->pageidx = pageidx;
  get_texturedict()->enter((unsigned long)newtexobj, (void *)obj);

  return newtexobj;
}

// *************************************************************************

void
CvrTextureObject::activateTexture(const SoGLRenderAction * action) const
{
  const GLuint texid = this->getGLTexture(action);

  const GLenum gltextypeenum = this->getGLTextureEnum();

  glEnable(gltextypeenum);
  glBindTexture(gltextypeenum, texid);

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
