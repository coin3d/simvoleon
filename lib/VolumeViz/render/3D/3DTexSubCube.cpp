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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include <VolumeViz/render/3D/Cvr3DTexSubCube.h>
#include <VolumeViz/render/common/CvrRGBATexture.h>
#include <VolumeViz/render/common/CvrPaletteTexture.h>
#include <VolumeViz/render/common/Cvr3DRGBATexture.h>
#include <VolumeViz/render/common/Cvr3DPaletteTexture.h>
#include <VolumeViz/misc/CvrCLUT.h>
#include <VolumeViz/misc/CvrVoxelChunk.h>
#include <VolumeViz/elements/SoTransferFunctionElement.h>

#include <Inventor/C/tidbits.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/projectors/SbPlaneProjector.h>

//#define HAVE_ARB_FRAGMENT_PROGRAM
// *************************************************************************

// debugging: keep this around until the peculiar NVidia bug with
// 1- or 2-pixel width textures has been analyzed. 20031031 mortene.

#define GLSYNCHRON 0

#if GLSYNCHRON
#define GLCMD(x) do { printf("%s\n", #x); x; glFinish(); } while(0)
#else
#define GLCMD(x) do { x; } while(0)
#endif

// *************************************************************************

unsigned int Cvr3DTexSubCube::nroftexels = 0;
unsigned int Cvr3DTexSubCube::texmembytes = 0;
GLuint Cvr3DTexSubCube::emptyimgname[1] = { 0 };
SbBool Cvr3DTexSubCube::detectedtextureswapping = FALSE;

// *************************************************************************

Cvr3DTexSubCube::Cvr3DTexSubCube(SoGLRenderAction * action,
                                 const CvrTextureObject * texobj,
                                 const SbVec3f & cubesize,
                                 const SbVec3s & originaltexsize,
                                 const SbBool compresstextures)
{
  this->bitspertexel = 0;
  this->clut = NULL;
  this->ispaletted = FALSE;

  assert(cubesize[0] >= 0);
  assert(cubesize[1] >= 0);
  assert(cubesize[2] >= 0);

  this->dimensions = cubesize;
  this->subcubestruct = NULL;
  this->origo = SbVec3f(0, 0, 0); // Default value

  // Actual dimensions of texture bitmap memory block.  
  this->texdims = SbVec3s(0, 0, 0);

  if (texobj->getTypeId() == Cvr3DPaletteTexture::getClassTypeId()) {
    this->texdims = ((Cvr3DPaletteTexture *) texobj)->dimensions;
  }
  else if (texobj->getTypeId() == Cvr3DRGBATexture::getClassTypeId()) {
    this->texdims = ((Cvr3DRGBATexture *) texobj)->dimensions;
  }
  else {
    assert(FALSE && "Cannot initialize a 3D texture cube with a 2D texture object");
  }


  this->compresstextures = compresstextures;
  const char * envstr = coin_getenv("CVR_COMPRESS_TEXTURES");
  if (envstr) { this->compresstextures = (compresstextures || atoi(envstr) > 0 ? TRUE : FALSE); }

  this->transferTex3GL(action, texobj);
  this->distancefromcamera = 0;
  this->originaltexsize = originaltexsize;

}


Cvr3DTexSubCube::~Cvr3DTexSubCube()
{
  if (this->texturename[0] != 0) {
    // FIXME: I'm pretty sure this is invoked outside of a GL context
    // from various places in the code. 20030306 mortene.
    GLCMD(glDeleteTextures(1, this->texturename)); 
  }

  if (this->clut) this->clut->unref();
}

SbBool
Cvr3DTexSubCube::isPaletted(void) const
{
  return this->ispaletted;
}

void
Cvr3DTexSubCube::setPalette(const CvrCLUT * newclut)
{
  if (this->clut) { this->clut->unref(); }
  this->clut = newclut;
  this->clut->ref();

  CvrCLUT * tmpclut = (CvrCLUT *) this->clut;
  tmpclut->setTextureType(CvrCLUT::TEXTURE3D);
}

// FIXME: Some magic has to be done to make this one work with OpenGL 1.0.
// torbjorv 08052002
void
Cvr3DTexSubCube::activateTexture(Interpolation interpolation) const
{

  GLCMD(glEnable(GL_TEXTURE_3D));

  if (this->texturename[0] == 0) {
    GLCMD(glBindTexture(GL_TEXTURE_3D, Cvr3DTexSubCube::emptyimgname[0]));
    return;
  }

  GLCMD(glBindTexture(GL_TEXTURE_3D, this->texturename[0]));
  
  GLenum interp = 0;
  switch (interpolation) {
  case NEAREST: interp = GL_NEAREST; break;
  case LINEAR: interp = GL_LINEAR; break;
  default: assert(FALSE); break;
  }

  GLCMD(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, interp));
  GLCMD(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, interp));

  assert(glGetError() == GL_NO_ERROR);

#if CVR_DEBUG && 0 // debug
  // FIXME: glAreTexturesResident() is OpenGL 1.1 only. 20021119 mortene.
  GLboolean residences[1];
  GLboolean resident = glAreTexturesResident(1, this->texturename, residences);
  if (!resident) {
    SoDebugError::postWarning("Cvr3DTexSubCube::activateTexture",
                              "texture %d not resident", this->texturename);
    Cvr3DTexSubCube::detectedtextureswapping = TRUE;
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


// If no palette specified, this function assumes RGBA data. If a
// palette is specified, the input data should be indices into the
// palette.  The function uses the palette's size to decide whether
// the indices are byte or short.
void
Cvr3DTexSubCube::transferTex3GL(SoGLRenderAction * action,
                                const CvrTextureObject * texobj)
{
  const cc_glglue * glw = cc_glglue_instance(action->getCacheContext());

  int colorformat;


  this->ispaletted = (texobj->getTypeId() == Cvr3DPaletteTexture::getClassTypeId());
  // only knows two types
  assert(this->ispaletted || texobj->getTypeId() == Cvr3DRGBATexture::getClassTypeId());

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
    this->clut = ((Cvr3DPaletteTexture *)texobj)->getCLUT();
    this->clut->ref();    
  }

  const int nrtexels = this->texdims[0] * this->texdims[1] * this->texdims[2];
  const int texmem = int(float(nrtexels) * float(this->bitspertexel) / 8.0f);

  // This is a debugging backdoor to test stuff with no limits on how
  // much texture memory we can use.

  // FIXME: there are no restrictions on texture memory yet -- it
  // needs support in other areas of the library. So until proper
  // resource handling is in place, anything goes. 20030324 mortene.

  static int unlimited_texmem = 1;
  if (unlimited_texmem == -1) {
    const char * envstr = coin_getenv("CVR_UNLIMITED_TEXMEM");
    if (envstr) { unlimited_texmem = atoi(envstr) > 0 ? 1 : 0; }
    else unlimited_texmem = 0;
  }


  // FIXME: limits should be stored in a global texture manager class
  // or some such. 20021121 mortene.
  if (!unlimited_texmem &&
      (//Cvr3DTexSubCube::detectedtextureswapping ||
       ((nrtexels + Cvr3DTexSubCube::nroftexels) > (16*1024*1024)) ||
       ((texmem + Cvr3DTexSubCube::texmembytes) > (64*1024*1024)))) {
#if CVR_DEBUG && 1 // debug
    static SbBool first = TRUE;
    if (first) {
      SoDebugError::postInfo("Cvr3DTexSubCube::transferTex2GL",
                             "filled up textures, nrtexels==%d, texmembytes==%d",
                             Cvr3DTexSubCube::nroftexels,
                             Cvr3DTexSubCube::texmembytes);
      first = FALSE;
    }
#endif // debug
    this->texturename[0] = 0;
  }
  else {
    Cvr3DTexSubCube::nroftexels += nrtexels;
    Cvr3DTexSubCube::texmembytes += texmem;

    // FIXME: these functions are only supported in opengl 1.1+...
    // torbjorv 08052002
    GLCMD(glGenTextures(1, this->texturename));
    assert(glGetError() == GL_NO_ERROR);

    GLCMD(glEnable(GL_TEXTURE_3D));
    GLCMD(glBindTexture(GL_TEXTURE_3D, this->texturename[0]));
    assert(glGetError() == GL_NO_ERROR);

    GLint wrapenum = GL_CLAMP;
    // FIXME: investigate if this is really what we want. 20021120 mortene.
    if (cc_glglue_has_texture_edge_clamp(glw)) { wrapenum = GL_CLAMP_TO_EDGE; }
    GLCMD(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, wrapenum));
    GLCMD(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, wrapenum));
    GLCMD(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, wrapenum));
    assert(glGetError() == GL_NO_ERROR);

    void * imgptr = NULL;
    if (this->ispaletted) imgptr = ((CvrPaletteTexture *)texobj)->getIndex8Buffer();
    else imgptr = ((CvrRGBATexture *)texobj)->getRGBABuffer();

    // FIXME: Combining texture compression and GL_COLOR_INDEX doesn't
    // seem to work on NVIDIA cards (tested on GeForceFX 5600 &
    // GeForce2 MX) (20040316 handegar)
    int palettetype = GL_COLOR_INDEX;

#ifdef HAVE_ARB_FRAGMENT_PROGRAM
      if (cc_glglue_has_arb_fragment_program(glw))
        palettetype = GL_LUMINANCE;     
#endif
      
    // FIXME: Is this way of compressing textures OK? (20040303 handegar)
    if (cc_glue_has_texture_compression(glw) && 
        this->compresstextures &&
        palettetype != GL_COLOR_INDEX) {
      if (colorformat == 4) colorformat = GL_COMPRESSED_RGBA_ARB;
      else colorformat = GL_COMPRESSED_INTENSITY_ARB;
    }
      
    cc_glglue_glTexImage3D(glw,
                           GL_TEXTURE_3D,
                           0,
                           colorformat,
                           this->texdims[0], this->texdims[1], this->texdims[2],
                           0,
                           this->ispaletted ? palettetype : GL_RGBA,
                           GL_UNSIGNED_BYTE,
                           imgptr);

    assert(glGetError() == GL_NO_ERROR);
    
  }

}

void
Cvr3DTexSubCube::activateCLUT(const SoGLRenderAction * action)
{
  assert(this->clut != NULL);

  // FIXME: should check if the same clut is already current 
  const cc_glglue * glw = cc_glglue_instance(action->getCacheContext());
  this->clut->activate(glw);
}

void
Cvr3DTexSubCube::deactivateCLUT(const SoGLRenderAction * action)
{
  assert(this->clut != NULL);

  // FIXME: should check if the same clut is already current 
  const cc_glglue * glw = cc_glglue_instance(action->getCacheContext());
  this->clut->deactivate(glw);
}

void * 
Cvr3DTexSubCube::subcube_clipperCB(const SbVec3f & v0, void * vdata0, 
                                   const SbVec3f & v1, void * vdata1,
                                   const SbVec3f & newvertex,
                                   void * userdata)
{
  
  Cvr3DTexSubCube * obj = (Cvr3DTexSubCube *) userdata;
  SbVec3f dist = SbVec3f(newvertex - obj->origo);
  
  const float tmp1 = obj->dimensions[0] + (obj->texdims[0] - obj->originaltexsize[0]);
  const float tmp2 = obj->dimensions[1] + (obj->texdims[1] - obj->originaltexsize[1]);
  const float tmp3 = obj->dimensions[2] + (obj->texdims[2] - obj->originaltexsize[2]);
  SbVec3f * texcoord = new SbVec3f(dist[0]/tmp1, dist[1]/tmp2, dist[2]/tmp3);
  
  obj->texcoordlist.append(texcoord);
  return (void *) texcoord;

}


// Check if this cube is intersected by the viewport aligned clip plane.
SbBool 
Cvr3DTexSubCube::checkIntersectionSlice(SbVec3f const & cubeorigo, 
                                        const SbViewVolume & viewvolume, 
                                        float viewdistance)
{
    
  SbClip cubeclipper(this->subcube_clipperCB, this);
  this->origo = cubeorigo;
  cubeclipper.reset();

  // FIXME: Can we rewrite this to support viewport shells for proper
  // perspective? (20040227 handegar)
  // FIXME: One should take aspect ratio into account here! (20040312 handegar)
  cubeclipper.addVertex(viewvolume.getPlanePoint(viewdistance, SbVec2f(-4.0f, 4.0f)));
  cubeclipper.addVertex(viewvolume.getPlanePoint(viewdistance, SbVec2f(4.0f, 4.0f)));
  cubeclipper.addVertex(viewvolume.getPlanePoint(viewdistance, SbVec2f(4.0f, -4.0f)));
  cubeclipper.addVertex(viewvolume.getPlanePoint(viewdistance, SbVec2f(-4.0f, -4.0f)));

  // ClockWise direction for all planes

  // Back plane
  cubeclipper.clip(SbPlane(cubeorigo + SbVec3f(0.0f, this->dimensions[1], 0.0f), 
                           cubeorigo,
                           cubeorigo + SbVec3f(this->dimensions[0], 0.0f, 0.0f))); 
  // Front plane
  cubeclipper.clip(SbPlane(cubeorigo + SbVec3f(this->dimensions[0], 0.0f, this->dimensions[2]),
                           cubeorigo + SbVec3f(0.0f, 0.0f, this->dimensions[2]),
                           cubeorigo + SbVec3f(0.0f, this->dimensions[1], this->dimensions[2])));
  // Bottom plane
  cubeclipper.clip(SbPlane(cubeorigo + SbVec3f(this->dimensions[0], 0.0f, 0.0f),
                           cubeorigo,
                           cubeorigo + SbVec3f(0.0f, 0.0f,  this->dimensions[2])));
  // Top plane
  cubeclipper.clip(SbPlane(cubeorigo + SbVec3f(0.0f, this->dimensions[1], this->dimensions[2]),
                           cubeorigo + SbVec3f(0.0f, this->dimensions[1], 0.0f),
                           cubeorigo + SbVec3f(this->dimensions[0], this->dimensions[1], 0.0f)));
  // Right plane
  cubeclipper.clip(SbPlane(cubeorigo + SbVec3f(this->dimensions[0], this->dimensions[1], 0.0f),
                           cubeorigo + SbVec3f(this->dimensions[0], 0.0f, 0.0f),
                           cubeorigo + SbVec3f(this->dimensions[0], 0.0f, this->dimensions[2])));
  // Left plane
  cubeclipper.clip(SbPlane(cubeorigo + SbVec3f(0.0f, 0.0f, this->dimensions[2]),
                           cubeorigo, 
                           cubeorigo + SbVec3f(0.0f, this->dimensions[1], 0.0f)));

  int i=0;
  const int result = cubeclipper.getNumVertices();
  if (result > 0) {
    subcube_slice slice;
    for (i=0;i<result;i++) {
      SbVec3f vert;
      cubeclipper.getVertex(i, vert);
      
      SbVec3f * tmp = (SbVec3f *) cubeclipper.getVertexData(i);
      // FIXME: Why does this happen? (20040317 handegar)
      if (tmp == NULL) continue; 

      SbVec3f texcoord(tmp->getValue());
      slice.vertex.append(vert);
      slice.texcoord.append(texcoord);
    }
    
    for (i=0;i<this->texcoordlist.getLength();++i)
      delete this->texcoordlist[i];
    
    this->texcoordlist.truncate(0);
    this->volumeslices.append(slice);
    return TRUE;
  }
  
  return FALSE;
}


void
Cvr3DTexSubCube::render(const SoGLRenderAction * action,
                        Interpolation interpolation)
{

  if (this->volumeslices.getLength() == 0) 
    return;

  // Texture binding/activation must happen before setting the
  // palette, or the previous palette will be used.
  this->activateTexture(interpolation);

  if (this->ispaletted) // Switch ON palette rendering
    this->activateCLUT(action);

  // FIXME: Maybe we should build a vertex array instead of making
  // glVertex3f calls. Would probably give a performance
  // gain. (20040312 handegar)
  
  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);    
  glEnable(GL_TEXTURE_3D);  

  for(int i=this->volumeslices.getLength()-1;i>=0;--i) {   
    
    glBegin(GL_TRIANGLE_FAN);
    for (int j=0;j<this->volumeslices[i].vertex.getLength(); ++j) {
      glTexCoord3fv(this->volumeslices[i].texcoord[j].getValue());
      glVertex3fv(this->volumeslices[i].vertex[j].getValue());      
    }
    glEnd();
           
    this->volumeslices[i].vertex.truncate(0);
    this->volumeslices[i].texcoord.truncate(0);

    assert(glGetError() == GL_NO_ERROR);

  }
  this->volumeslices.truncate(0);


  if (this->ispaletted) // Switch OFF palette rendering
    this->deactivateCLUT(action);

}

// For debugging purposes
void
Cvr3DTexSubCube::renderBBox(const SoGLRenderAction * action, int counter)
{
 
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_TEXTURE_3D);
  glDisable(GL_BLEND);

  glBegin(GL_LINE_LOOP);
  glVertex3fv(this->origo.getValue());
  glVertex3fv((this->origo + SbVec3f(this->dimensions[0], 0, 0)).getValue());
  glVertex3fv((this->origo + SbVec3f(this->dimensions[0], this->dimensions[1], 0)).getValue());
  glVertex3fv((this->origo + SbVec3f(0, this->dimensions[1], 0)).getValue());
  glVertex3fv(this->origo.getValue());
  glVertex3fv((this->origo + SbVec3f(0, 0, this->dimensions[2])).getValue());
  glVertex3fv((this->origo + SbVec3f(this->dimensions[0], 0, this->dimensions[2])).getValue());
  glVertex3fv((this->origo + SbVec3f(this->dimensions[0], this->dimensions[1], this->dimensions[2])).getValue());
  glVertex3fv((this->origo + SbVec3f(0, this->dimensions[1], this->dimensions[2])).getValue());
  glVertex3fv((this->origo + SbVec3f(0, 0, this->dimensions[2])).getValue());
  glEnd();

}


float
Cvr3DTexSubCube::getDistanceFromCamera()
{
  return this->distancefromcamera;
}

void
Cvr3DTexSubCube::setDistanceFromCamera(float dist)
{
  this->distancefromcamera = dist;
}


unsigned int
Cvr3DTexSubCube::totalNrOfTexels(void)
{
  return Cvr3DTexSubCube::nroftexels;
}

unsigned int
Cvr3DTexSubCube::totalTextureMemoryUsed(void)
{
  return Cvr3DTexSubCube::texmembytes;
}
