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

#include <VolumeViz/render/2D/Cvr2DTexSubPage.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include <Inventor/C/glue/gl.h>
#include <Inventor/C/tidbits.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/nodes/SoDrawStyle.h>

#include <VolumeViz/elements/CvrCompressedTexturesElement.h>
#include <VolumeViz/elements/SoTransferFunctionElement.h>
#include <VolumeViz/misc/CvrCLUT.h>
#include <VolumeViz/misc/CvrUtil.h>
#include <VolumeViz/misc/CvrVoxelChunk.h>
#include <VolumeViz/render/common/Cvr2DPaletteTexture.h>
#include <VolumeViz/render/common/Cvr2DRGBATexture.h>

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

GLuint Cvr2DTexSubPage::emptyimgname[1] = { 0 };

// *************************************************************************

Cvr2DTexSubPage::Cvr2DTexSubPage(const SoGLRenderAction * action,
                                 const CvrTextureObject * texobj,
                                 const SbVec2s & pagesize,
                                 const SbVec2s & texsize)
{
  this->bitspertexel = 0;
  this->clut = NULL;

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
  const SbVec2s texdims(dims[0], dims[1]);

  // Calculates part of texture to show.
  this->texmaxcoords = SbVec2f(1.0f, 1.0f);
  if (texdims != texsize) {
    this->texmaxcoords[0] = float(texsize[0]) / float(texdims[0]);
    this->texmaxcoords[1] = float(texsize[1]) / float(texdims[1]);
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
}

Cvr2DTexSubPage::~Cvr2DTexSubPage()
{
  this->texobj->unref();
  if (this->clut) this->clut->unref();
}

// *************************************************************************

SbBool
Cvr2DTexSubPage::isPaletted(void) const
{
  return this->texobj->isPaletted();
}

void
Cvr2DTexSubPage::setPalette(const CvrCLUT * newclut)
{
  if (this->clut) { this->clut->unref(); }
  this->clut = newclut;
  newclut->ref();
}

// *************************************************************************

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

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#endif
}

// *************************************************************************

// FIXME: this is common with 3DTexSubCube -- move to
// CvrTextureObject? 20040720 mortene.

void
Cvr2DTexSubPage::activateCLUT(const SoGLRenderAction * action)
{
  assert(this->clut != NULL);

  // FIXME: should check if the same clut is already current 
  this->clut->activate(action->getCacheContext(), CvrCLUT::TEXTURE2D);
}


void
Cvr2DTexSubPage::deactivateCLUT(const SoGLRenderAction * action)
{
  assert(this->clut != NULL);

  // FIXME: should check if the same clut is already current 
  const cc_glglue * glw = cc_glglue_instance(action->getCacheContext());
  this->clut->deactivate(glw);
}

// *************************************************************************

void
Cvr2DTexSubPage::renderQuad(const SoGLRenderAction * action,
                            const SbVec3f & upleft,
                            const SbVec3f & lowleft,
                            const SbVec3f & upright,
                            const SbVec3f & lowright)
{
  // Texture binding/activation must happen before setting the
  // palette, or the previous palette will be used.
  this->texobj->activateTexture(action);
  if (this->isPaletted()) { this->activateCLUT(action); }

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

  // Switch OFF palette rendering
  if (this->isPaletted()) { this->deactivateCLUT(action); }
}

void
Cvr2DTexSubPage::render(const SoGLRenderAction * action,
                        const SbVec3f & upleft,
                        SbVec3f widthvec, SbVec3f heightvec)
{

 
  // 0: as usual, 1: with slice wireframes, 2: only wireframes
  unsigned int renderstyle = CvrUtil::debugRenderStyle();

  // Shall we draw the oblique slice as lines/wireframe?
  SoDrawStyleElement::Style drawstyle = SoDrawStyleElement::get(action->getState());
  if (drawstyle == SoDrawStyleElement::LINES) renderstyle = 2;
  
  // Scale span of GL quad to match the visible part of the
  // texture. (Border subpages shouldn't show all of the texture, if
  // the dimensions of the dataset are not a power of two, or if the
  // dimensions are less than the subpage size).

  widthvec *= this->quadpartfactors[0];
  heightvec *= this->quadpartfactors[1];

  // Find all corner points of the quad.

  const SbVec3f lowleft = upleft + heightvec;
  const SbVec3f lowright = lowleft + widthvec;
  const SbVec3f upright = upleft + widthvec;

  if (renderstyle != 2) {
    this->renderQuad(action, upleft, lowleft, upright, lowright);
  }
  
  if (renderstyle != 0) {
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_LINE_LOOP);
    glVertex3f(lowleft[0], lowleft[1], lowleft[2]);
    glVertex3f(lowright[0], lowright[1], lowright[2]);
    glVertex3f(upright[0], upright[1], upright[2]);
    glVertex3f(upleft[0], upleft[1], upleft[2]);
    glEnd();
  }
}
