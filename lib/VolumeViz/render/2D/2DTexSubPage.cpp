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

#include <VolumeViz/render/2D/Cvr2DTexSubPage.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include <Inventor/C/glue/gl.h>
#include <Inventor/C/tidbits.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/errors/SoDebugError.h>

#include <VolumeViz/elements/SoTransferFunctionElement.h>
#include <VolumeViz/misc/CvrCLUT.h>
#include <VolumeViz/misc/CvrUtil.h>
#include <VolumeViz/misc/CvrVoxelChunk.h>
#include <VolumeViz/render/common/Cvr2DPaletteTexture.h>
#include <VolumeViz/render/common/Cvr2DRGBATexture.h>
#include <VolumeViz/render/common/resourcehandler.h>

#include "texmemfullimg.h"

// *************************************************************************

// FIXME: debugging, keep this around until the peculiar NVidia bug with
// 1- or 2-pixel width textures has been analyzed. 20031031 mortene.

// How to use this: just wrap the GL suspected of misdoings in
//
// GLCMD(glSomething(arg0, arg1, etc));
//
// ..and set the GLSYNCHRON define below to "1". Anything causing the
// GL driver to crash will then do so synchronously due to the
// glFinish() call.

#define GLSYNCHRON 0

#if GLSYNCHRON
#define GLCMD(x) do { printf("%s\n", #x); x; glFinish(); } while(0)
#else
#define GLCMD(x) do { x; } while(0)
#endif

// *************************************************************************

unsigned int Cvr2DTexSubPage::nroftexels = 0;
unsigned int Cvr2DTexSubPage::texmembytes = 0;
GLuint Cvr2DTexSubPage::emptyimgname[1] = { 0 };
SbBool Cvr2DTexSubPage::detectedtextureswapping = FALSE;

// *************************************************************************

// Callback function for the GL resource handler.
SbBool
Cvr2DTexSubPage::resourceCleanerS(void * owner, uint32_t ctxid, void * resource, void * closure)
{
  struct Cvr2DTexSubPage::GLResource * res = (struct Cvr2DTexSubPage::GLResource *)resource;
  glDeleteTextures(1, &(res->texid));
  delete res;
  return TRUE;
}

// *************************************************************************

// FIXME: first argument should be const. 20040716 mortene.
Cvr2DTexSubPage::Cvr2DTexSubPage(SoGLRenderAction * action,
                                 const CvrTextureObject * texobj,
                                 const SbVec2s & pagesize,
                                 const SbVec2s & texsize,
                                 // FIXME: this should be on the state
                                 // stack. 20040716 mortene.
                                 const SbBool compresstextures)
{
  // We're using the GL resource handler, so plug in our deletion
  // callback.
  cvr_rc_add_deletion_func(this, Cvr2DTexSubPage::resourceCleanerS, NULL);

  this->bitspertexel = 0;
  this->clut = NULL;
  this->ispaletted = FALSE;

  assert(pagesize[0] >= 0);
  assert(pagesize[1] >= 0);
  assert(coin_is_power_of_two(pagesize[0]));
  assert(coin_is_power_of_two(pagesize[1]));

  assert(texsize[0] <= pagesize[0]);
  assert(texsize[1] <= pagesize[1]);

  this->texobj = texobj;
  this->texobj->ref();

  const SbVec3s dims = this->texobj->getDimensions();
  assert((dims[2] == 1) && "Cannot initialize a 2D-texture subpage with a 3D-texture object");
  this->texdims.setValue(dims[0], dims[1]);

  // Calculates part of texture to show.
  this->texmaxcoords = SbVec2f(1.0f, 1.0f);
  if (this->texdims != texsize) {
    this->texmaxcoords[0] = float(texsize[0]) / float(this->texdims[0]);
    this->texmaxcoords[1] = float(texsize[1]) / float(this->texdims[1]);
  }

  // Calculates part of GL quad to show.
  this->quadpartfactors = SbVec2f(1.0f, 1.0f);
  if (pagesize != texsize) {
    this->quadpartfactors[0] = float(texsize[0]) / float(pagesize[0]);
    this->quadpartfactors[1] = float(texsize[1]) / float(pagesize[1]);
  }

#if CVR_DEBUG && 0 // debug
  SoDebugError::postInfo("Cvr2DTexSubPage::Cvr2DTexSubPage",
                         "texsize==[%d, %d], "
                         "texobj->getDimensions()==[%d, %d], "
                         "this->texmaxcoords==[%f, %f]",
                         texsize[0], texsize[1],
                         this->texobj->getDimensions()[0],
                         this->texobj->getDimensions()[1],
                         this->texmaxcoords[0], this->texmaxcoords[1]);
#endif // debug

  this->compresstextures = compresstextures;
  const char * envstr = coin_getenv("CVR_COMPRESS_TEXTURES");
  if (envstr) { this->compresstextures = (compresstextures || atoi(envstr) > 0 ? TRUE : FALSE); }
}

Cvr2DTexSubPage::~Cvr2DTexSubPage()
{
  this->texobj->unref();

  // Notify GL resource handler that all our resources should be
  // tagged dead, and destructed the next time the GL context is made
  // current.
  cvr_rc_tag_resources_dead(this);

  // FIXME: move resource usage information to resource handler
  // (cvr_rc_*) interface. 20040715 mortene.
#if 0 // disabled
  const unsigned int nrtexels = this->texdims[0] * this->texdims[1];
  assert(nrtexels <= Cvr2DTexSubPage::nroftexels);
  Cvr2DTexSubPage::nroftexels -= nrtexels;

  unsigned int freetexmem = (unsigned int)
    (float(nrtexels) * float(this->bitspertexel) / 8.0f);
  assert(freetexmem <= Cvr2DTexSubPage::texmembytes);
  Cvr2DTexSubPage::texmembytes -= freetexmem;

#if CVR_DEBUG && 0 // debug
  SoDebugError::postInfo("Cvr2DTexSubPage::~Cvr2DTexSubPage",
                         "nroftexels => %d, texmembytes => %d",
                         Cvr2DTexSubPage::nroftexels,
                         Cvr2DTexSubPage::texmembytes);
#endif // debug
#endif // disabled

  if (this->clut) this->clut->unref();
}

// *************************************************************************

SbBool
Cvr2DTexSubPage::isPaletted(void) const
{
  return this->ispaletted;
}

void
Cvr2DTexSubPage::setPalette(const CvrCLUT * newclut)
{
  if (this->clut) { this->clut->unref(); }
  this->clut = newclut;
  newclut->ref();
}

// *************************************************************************

void
Cvr2DTexSubPage::activateTexture(const SoGLRenderAction * action,
                                 Interpolation interpolation) const
{
  uint32_t glctxid = action->getCacheContext();
  void * resource;
  const SbBool found = cvr_rc_find_resource(glctxid, (void *)this, &resource);

  struct Cvr2DTexSubPage::GLResource * res = NULL;
  if (!found) { 
    // FIXME: makeGLTexture() should really be const. 20040715 mortene.
    res = ((Cvr2DTexSubPage *)this)->makeGLTexture(action);
  }
  else {
    res = (struct Cvr2DTexSubPage::GLResource *)resource;
  }

  if (res == NULL) {
    // Texture could not be found nor made. A warning should have been
    // displayed somewhere deeper down in the call-chain, so we simply
    // return.
    return;
  }

  glBindTexture(GL_TEXTURE_2D, res->texid);

  GLenum interp = 0;
  switch (interpolation) {
  case NEAREST: interp = GL_NEAREST; break;
  case LINEAR: interp = GL_LINEAR; break;
  default: assert(FALSE); break;
  }

  // FIXME: why are we using float version of glTexParameter here? All
  // arguments looks like ints..? 20031027 mortene.
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, interp);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, interp);
  assert(glGetError() == GL_NO_ERROR);

#if CVR_DEBUG && 0 // debug
  // FIXME: glAreTexturesResident() is OpenGL 1.1 only. 20021119 mortene.
  GLboolean residences[1];
  GLboolean resident = glAreTexturesResident(1, &(res->texid), residences);
  if (!resident) {
    SoDebugError::postWarning("Cvr2DTexSubPage::activateTexture",
                              "texture %d not resident", res->texid);
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

// Set up the image used (for debugging purposes) when texture memory
// is full.
void
Cvr2DTexSubPage::bindTexMemFullImage(const cc_glglue * glw)
{
  // FIXME: emptyimgname not yet bound to a specific context. Should
  // anyway find a better way to flag that no texture can be
  // made. 20040715 mortene.
#if 0 // disabled
  // FIXME: requires > OpenGL 1.0, should go through wrapper.
  // FIXME: bind to specific context. 20040715 mortene.
  glGenTextures(1, Cvr2DTexSubPage::emptyimgname);
  glBindTexture(GL_TEXTURE_2D, Cvr2DTexSubPage::emptyimgname[0]);
  // FIXME: never freed. 20021121 mortene.
    
  // Check format of GIMP-exported "texmem full" image.
  assert(coin_is_power_of_two(tex_image.width));
  assert(coin_is_power_of_two(tex_image.height));
  assert(tex_image.bytes_per_pixel == 4);

  glTexImage2D(GL_TEXTURE_2D,
               0,
               tex_image.bytes_per_pixel,
               tex_image.width, tex_image.height,
               0,
               GL_RGBA,
               GL_UNSIGNED_BYTE,
               tex_image.pixel_data);

  const int texels = tex_image.width * tex_image.height;
  Cvr2DTexSubPage::nroftexels += texels;
  Cvr2DTexSubPage::texmembytes += texels * tex_image.bytes_per_pixel;

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#endif
}

// If no palette specified, this function assumes RGBA data. If a
// palette is specified, the input data should be indices into the
// palette.  The function uses the palette's size to decide whether
// the indices are byte or short.
//
// FIXME: this function should be const. 20040715 mortene.
struct Cvr2DTexSubPage::GLResource *
Cvr2DTexSubPage::makeGLTexture(const SoGLRenderAction * action)
{
  const uint32_t glctx = action->getCacheContext();
  const cc_glglue * glw = cc_glglue_instance(glctx);

  // FIXME: emptyimgname not yet bound to a specific context. Should
  // anyway find a better way to flag that no texture can be
  // made. 20040715 mortene.
#if 0 // disabled
  if (Cvr2DTexSubPage::emptyimgname[0] == 0) {
    Cvr2DTexSubPage::bindTexMemFullImage(glw);
  }
#endif

  int colorformat;

  this->ispaletted = this->texobj->getTypeId() == Cvr2DPaletteTexture::getClassTypeId();
  // only knows two types
  assert(this->ispaletted || this->texobj->getTypeId() == Cvr2DRGBATexture::getClassTypeId());

  // For uploading standard RGBA-texture
  if (!this->ispaletted) {
    colorformat = 4;
    this->bitspertexel = 32; // 8 bits each R, G, B & A
  }
  // For uploading paletted texture
  else {
    colorformat = GL_COLOR_INDEX8_EXT;
    this->bitspertexel = 8;
    if (this->clut) this->clut->unref();
    this->clut = ((CvrPaletteTexture *)this->texobj)->getCLUT();
    this->clut->ref();
  }

  const int nrtexels = this->texdims[0] * this->texdims[1];
  const int texmem = int(float(nrtexels) * float(this->bitspertexel) / 8.0f);

  // This is a debugging backdoor to test stuff with no limits on how
  // much texture memory we can use.

  // FIXME: there are no restrictions on texture memory yet -- it
  // needs support in other areas of the library. So until proper
  // resource handling is in place, anything goes. 20030324 mortene.

//   static int unlimited_texmem = -1;
  static int unlimited_texmem = 1;
  if (unlimited_texmem == -1) {
    const char * envstr = coin_getenv("CVR_UNLIMITED_TEXMEM");
    if (envstr) { unlimited_texmem = atoi(envstr) > 0 ? 1 : 0; }
    else unlimited_texmem = 0;
  }


  // FIXME: limits should be stored in a global texture manager class
  // or some such. 20021121 mortene.
  if (!unlimited_texmem &&
      (//Cvr2DTexSubPage::detectedtextureswapping ||
       ((nrtexels + Cvr2DTexSubPage::nroftexels) > (16*1024*1024)) ||
       ((texmem + Cvr2DTexSubPage::texmembytes) > (64*1024*1024)))) {
#if CVR_DEBUG && 1 // debug
    static SbBool first = TRUE;
    if (first) {
      SoDebugError::postInfo("Cvr2DTexSubPage::makeGLTexture",
                             "filled up textures, nrtexels==%d, texmembytes==%d",
                             Cvr2DTexSubPage::nroftexels,
                             Cvr2DTexSubPage::texmembytes);
      first = FALSE;
    }
#endif // debug

    return NULL;
  }


  Cvr2DTexSubPage::nroftexels += nrtexels;
  Cvr2DTexSubPage::texmembytes += texmem;

  // FIXME: glGenTextures() / glBindTexture() / glDeleteTextures()
  // are only supported in opengl >= 1.1, should have a fallback for
  // 1.0 drivers, like we have in Coin, where we can use display
  // lists instead. 20040715 mortene.

  struct Cvr2DTexSubPage::GLResource * res = new struct Cvr2DTexSubPage::GLResource;
  glGenTextures(1, &(res->texid));
  assert(glGetError() == GL_NO_ERROR);

  cvr_rc_bind_resource(glctx, this, res);

  glBindTexture(GL_TEXTURE_2D, res->texid);
  assert(glGetError() == GL_NO_ERROR);

  void * imgptr = NULL;
  if (this->ispaletted) imgptr = ((Cvr2DPaletteTexture *)this->texobj)->getIndex8Buffer();
  else imgptr = ((Cvr2DRGBATexture *)this->texobj)->getRGBABuffer();

  // debugging: keep this around until the peculiar NVidia bug with
  // 1- or 2-pixel width textures has been analyzed.
  //
  // FIXME: need to find out what the hell this problem is. 20031031 mortene.
#if 0
  printf("glTexImage2D() args: dims==<%d,%d>, ispaletted==%d, imgptr==%p\n",
         this->texdims[0], this->texdims[1], this->ispaletted, imgptr);

  static int allnum = 0;
  allnum++;
  if (this->texdims[0]==1 || this->texdims[1]==1) {
    static int num = 0;
    num++;
    printf("numero %d/%d, nroftexels==%d, texmembytes==%d\n",
           num, allnum,
           Cvr2DTexSubPage::nroftexels,
           Cvr2DTexSubPage::texmembytes);
  }
  else {
    printf("numero %d, nroftexels==%d, texmembytes==%d\n",
           allnum,
           Cvr2DTexSubPage::nroftexels,
           Cvr2DTexSubPage::texmembytes);
  }
#endif // debugging
   

  // FIXME: Combining texture compression and GL_COLOR_INDEX doesnt
  // seem to work on NVIDIA cards (tested on GeForceFX 5600 &
  // GeForce2 MX) (20040316 handegar)
  int palettetype = GL_COLOR_INDEX;

#ifdef HAVE_ARB_FRAGMENT_PROGRAM
  if (cc_glglue_has_arb_fragment_program(glw))
    palettetype = GL_LUMINANCE;    
#endif // HAVE_ARB_FRAGMENT_PROGRAM

  // FIXME: Is this way of compressing textures OK? (20040303 handegar)
  if (cc_glue_has_texture_compression(glw) && 
      this->compresstextures &&
      palettetype != GL_COLOR_INDEX) {
    if (colorformat == 4) colorformat = GL_COMPRESSED_RGBA_ARB;
    else colorformat = GL_COMPRESSED_INTENSITY_ARB;
  }

  if (!CvrUtil::dontModulateTextures()) // Is texture mod. disabled by an envvar?
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
  glTexImage2D(GL_TEXTURE_2D,
               0,
               colorformat,
               this->texdims[0],
               this->texdims[1],
               0,
               this->ispaletted ? palettetype : GL_RGBA,
               GL_UNSIGNED_BYTE,
               imgptr);
                 
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

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapenum);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapenum);
  assert(glGetError() == GL_NO_ERROR);

  return res;
}

void
Cvr2DTexSubPage::activateCLUT(const SoGLRenderAction * action)
{
  assert(this->clut != NULL);

  // FIXME: should check if the same clut is already current 
  const cc_glglue * glw = cc_glglue_instance(action->getCacheContext());
  this->clut->activate(glw);
}


void
Cvr2DTexSubPage::deactivateCLUT(const SoGLRenderAction * action)
{
  assert(this->clut != NULL);

  // FIXME: should check if the same clut is already current 
  const cc_glglue * glw = cc_glglue_instance(action->getCacheContext());
  this->clut->deactivate(glw);
}


void
Cvr2DTexSubPage::render(const SoGLRenderAction * action,
                        const SbVec3f & upleft,
                        SbVec3f widthvec, SbVec3f heightvec,
                        Interpolation interpolation)
{

  // Texture binding/activation must happen before setting the
  // palette, or the previous palette will be used.
  this->activateTexture(action, interpolation);
  if (this->ispaletted) { this->activateCLUT(action); }

  // Scale span of GL quad to match the visible part of the
  // texture. (Border subpages shouldn't show all of the texture, if
  // the dimensions of the dataset are not a power of two, or if the
  // dimensions are less than the subpage size).

  widthvec *= this->quadpartfactors[0];
  heightvec *= this->quadpartfactors[1];

  // Find all corner points of the quad.

  SbVec3f lowleft = upleft + heightvec;
  SbVec3f lowright = lowleft + widthvec;
  SbVec3f upright = upleft + widthvec;

  if (CvrUtil::dontModulateTextures()) // Is texture mod. disabled by an envvar?
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

  // Texturecoords are set up so the texture is flipped in the
  // Y-direction, as the volume data and texture map data are oriented
  // in the opposite direction (top-to-bottom) from what the Y axis in
  // the OpenGL coordinate system uses (bottom-to-top).

  glBegin(GL_QUADS);

  glTexCoord2f(0.0f, this->texmaxcoords[1]);
  glVertex3f(lowleft[0], lowleft[1], lowleft[2]);

  glTexCoord2f(this->texmaxcoords[0], this->texmaxcoords[1]);
  glVertex3f(lowright[0], lowright[1], lowright[2]);

  glTexCoord2f(this->texmaxcoords[0], 0.0f);
  glVertex3f(upright[0], upright[1], upright[2]);

  glTexCoord2f(0.0f, 0.0f);
  glVertex3f(upleft[0], upleft[1], upleft[2]);

  glEnd();


  if (this->ispaletted) // Switch OFF palette rendering
    this->deactivateCLUT(action);

  // This is a debugging backdoor: if the environment variable
  // CVR_SUBPAGE_FRAMES is set to a positive integer, a lineset will
  // be drawn around the border of each page.

  static int showsubpageframes = -1;
  if (showsubpageframes == -1) {
    const char * envstr = coin_getenv("CVR_SUBPAGE_FRAMES");
    if (envstr) { showsubpageframes = atoi(envstr) > 0 ? 1 : 0; }
    else showsubpageframes = 0;
  }

  if (showsubpageframes) {
    glDisable(GL_TEXTURE_2D);
    glLineStipple(1, 0xffff);
    glLineWidth(2);

    glBegin(GL_LINE_LOOP);
    glVertex3f(lowleft[0], lowleft[1], lowleft[2]);
    glVertex3f(lowright[0], lowright[1], lowright[2]);
    glVertex3f(upright[0], upright[1], upright[2]);
    glVertex3f(upleft[0], upleft[1], upleft[2]);
    glEnd();

    glEnable(GL_TEXTURE_2D);
  }

}

unsigned int
Cvr2DTexSubPage::totalNrOfTexels(void)
{
  return Cvr2DTexSubPage::nroftexels;
}

unsigned int
Cvr2DTexSubPage::totalTextureMemoryUsed(void)
{
  return Cvr2DTexSubPage::texmembytes;
}
