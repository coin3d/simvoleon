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
#include <Inventor/C/glue/gl.h>
#include <Inventor/C/tidbits.h>
#include <Inventor/SbName.h>
#include <Inventor/actions/SoGLRenderAction.h>

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
  this->iscompressed = FALSE;
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

  glGenTextures(1, &texid);
  assert(glGetError() == GL_NO_ERROR);

  glEnable(GL_TEXTURE_3D);
  glBindTexture(GL_TEXTURE_3D, texid);
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
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, wrapenum);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, wrapenum);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, wrapenum);
  assert(glGetError() == GL_NO_ERROR);

  void * imgptr = NULL;
  if (this->isPaletted()) imgptr = ((CvrPaletteTexture *)this)->getIndex8Buffer();
  else imgptr = ((CvrRGBATexture *)this)->getRGBABuffer();


  int palettetype = GL_COLOR_INDEX;
#ifdef HAVE_ARB_FRAGMENT_PROGRAM
  if (cc_glglue_has_arb_fragment_program(glw)) { palettetype = GL_LUMINANCE; }
#endif // HAVE_ARB_FRAGMENT_PROGRAM

  // NOTE: Combining texture compression and GL_COLOR_INDEX doesn't
  // seem to work on NVIDIA cards (tested on GeForceFX 5600 &
  // GeForce2 MX) (20040316 handegar)
  if (cc_glue_has_texture_compression(glw) &&
      this->textureCompressed() &&
      palettetype != GL_COLOR_INDEX) {
    if (colorformat == 4) colorformat = GL_COMPRESSED_RGBA_ARB;
    else colorformat = GL_COMPRESSED_INTENSITY_ARB;
  }

  if (!CvrUtil::dontModulateTextures()) // Is texture modulation disabled by an envvar?
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  cc_glglue_glTexImage3D(glw,
                         GL_TEXTURE_3D,
                         0,
                         colorformat,
                         texdims[0], texdims[1], texdims[2],
                         0,
                         this->isPaletted() ? palettetype : GL_RGBA,
                         GL_UNSIGNED_BYTE,
                         imgptr);

  assert(glGetError() == GL_NO_ERROR);

  cvr_rc_bind_resource(glctxid, (void *)this, (void *)texid);

  return texid;
}

// *************************************************************************

// FIXME: needs to go. 20040716 mortene.

void
CvrTextureObject::setTextureCompressed(SbBool flag)
{
  this->iscompressed = flag;
}

SbBool
CvrTextureObject::textureCompressed() const
{
  return this->iscompressed;
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

// FIXME: the code below this point has been moved over from
// CvrTextureManager, and should be fixed up. 20040716 mortene.

enum TEXTURETYPE {
  TEXTURE2D,
  TEXTURE3D
};

// FIXME: remove this -- write comparison functions into
// CvrTextureObject. 20040716 mortene.

struct textureobj {
  uint32_t voldataid; // This will change if volume-data has been modified by the user.
  TEXTURETYPE texturetype;
  SbBox3s cutcube;
  SbBox2s cutpage;
  CvrTextureObject * object;
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
find_textureobject_3D(const uint32_t voldataid, const SbBox3s cutcube)
{
  SbPList keylist;
  SbPList valuelist;
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
        (obj->texturetype == TEXTURE3D))
      return obj;
  }

  return NULL;
}

const CvrTextureObject *
CvrTextureObject::create(const SoGLRenderAction * action,
                         const SbVec3s & texsize,
                         const SbBox3s & cutcube)
{
  SoState * state = action->getState();
  const SoVolumeDataElement * volumedataelement = SoVolumeDataElement::getInstance(state);
  const SoVolumeData * voldata = volumedataelement->getVolumeData();

  textureobj * obj = find_textureobject_3D(voldata->getNodeId(), cutcube);

  if (obj != NULL) {
    assert((obj->texturetype == TEXTURE3D) && "Type != TEXTURE3D. Invalid texture type!");
    // FIXME: should ref() in caller, not here. 20040716 mortene.
    obj->object->ref();
    return obj->object;
  }

  CvrTextureObject * newobj =
    CvrTextureObject::new3DTextureObject(action, voldata, texsize, cutcube);
  obj = new textureobj;
  obj->object = newobj;
  obj->voldataid = voldata->getNodeId();
  obj->cutcube = cutcube;
  obj->texturetype = TEXTURE3D;
  get_texturedict()->enter((unsigned long) newobj, (void *) obj);
  newobj->ref();

  return newobj;
}

CvrTextureObject *
CvrTextureObject::new3DTextureObject(const SoGLRenderAction * action,
                                     const SoVolumeData * voldata,
                                     const SbVec3s & texsize,
                                     const SbBox3s & cutcube)
{

  SbVec3s vddims;
  void * dataptr;
  SoVolumeData::DataType type;
  SbBool ok = voldata->getVolumeData(vddims, dataptr, type);
  assert(ok);

  CvrVoxelChunk::UnitSize vctype;
  switch (type) {
  case SoVolumeData::UNSIGNED_BYTE: vctype = CvrVoxelChunk::UINT_8; break;
  case SoVolumeData::UNSIGNED_SHORT: vctype = CvrVoxelChunk::UINT_16; break;
  default: assert(FALSE); break;
  }

  CvrVoxelChunk * input = new CvrVoxelChunk(vddims, vctype, dataptr);
  CvrVoxelChunk * cubechunk = input->buildSubCube(cutcube);
  delete input;

  SbBool invisible = FALSE;
  CvrTextureObject * newtexobj = cubechunk->transfer3D(action, invisible);

  if (invisible) // Is the cubechunk completely transparent?
    return NULL; // FIXME: NULL return is not handled in caller. 20040716 mortene.

  // Must clear unused texture area to prevent artifacts due to
  // floating point inaccuracies when calculating texture coords.
  newtexobj->blankUnused(texsize);
  const SbVec3s realtexsize = newtexobj->getDimensions();

  // FIXME: should be passed on the stack. Should also only be
  // considered when generating GL textures. 20040716 mortene.
  newtexobj->setTextureCompressed(voldata->useCompressedTexture.getValue());

  return newtexobj;
}

// *************************************************************************
