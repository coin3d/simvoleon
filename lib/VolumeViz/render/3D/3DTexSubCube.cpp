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

#include <VolumeViz/render/3D/Cvr3DTexSubCube.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include <Inventor/C/glue/gl.h>
#include <Inventor/C/tidbits.h>
#include <Inventor/SbClip.h>
#include <Inventor/SbColor.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/elements/SoLazyElement.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/projectors/SbPlaneProjector.h>

#include <VolumeViz/elements/SoTransferFunctionElement.h>
#include <VolumeViz/misc/CvrCLUT.h>
#include <VolumeViz/misc/CvrUtil.h>
#include <VolumeViz/render/common/Cvr3DPaletteTexture.h>
#include <VolumeViz/render/common/Cvr3DRGBATexture.h>
#include <VolumeViz/render/common/CvrPaletteTexture.h>
#include <VolumeViz/render/common/CvrRGBATexture.h>

// *************************************************************************

Cvr3DTexSubCube::Cvr3DTexSubCube(const SoGLRenderAction * action,
                                 const CvrTextureObject * texobj,
                                 const SbVec3f & cubesize,
                                 const SbVec3s & originaltexsize)
{
  this->clut = NULL;

  assert(cubesize[0] >= 0);
  assert(cubesize[1] >= 0);
  assert(cubesize[2] >= 0);

  this->dimensions = cubesize;
  this->origo = SbVec3f(0, 0, 0); // Default value

  if (texobj->getTypeId() == Cvr3DPaletteTexture::getClassTypeId()) {
    this->clut = ((CvrPaletteTexture *)texobj)->getCLUT();
    this->clut->ref();
  }

  this->textureobject = texobj;
  this->textureobject->ref();

  this->distancefromcamera = 0;
  this->originaltexsize = originaltexsize;
}

Cvr3DTexSubCube::~Cvr3DTexSubCube()
{
  this->textureobject->unref();

  if (this->clut) this->clut->unref();
}

// *************************************************************************

SbBool
Cvr3DTexSubCube::isPaletted(void) const
{
  return this->textureobject->isPaletted();
}

void
Cvr3DTexSubCube::setPalette(const CvrCLUT * newclut)
{
  assert(newclut != NULL);

  if (this->clut) { this->clut->unref(); }
  this->clut = newclut;
  this->clut->ref();

  ((CvrCLUT *) this->clut)->setTextureType(CvrCLUT::TEXTURE3D);
}

// *************************************************************************

// FIXME: almost identical with 2DTexSubPage's ditto, should be
// possible to share. 20040719 mortene.

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

// *************************************************************************

void *
Cvr3DTexSubCube::subcube_clipperCB(const SbVec3f & v0, void * vdata0,
                                   const SbVec3f & v1, void * vdata1,
                                   const SbVec3f & newvertex,
                                   void * userdata)
{

  Cvr3DTexSubCube * obj = (Cvr3DTexSubCube *) userdata;
  SbVec3f dist = SbVec3f(newvertex - obj->origo);

  const SbVec3s texdims = obj->textureobject->getDimensions();

  const float tmp1 = obj->dimensions[0] + (texdims[0] - obj->originaltexsize[0]);
  const float tmp2 = obj->dimensions[1] + (texdims[1] - obj->originaltexsize[1]);
  const float tmp3 = obj->dimensions[2] + (texdims[2] - obj->originaltexsize[2]);

  SbVec3f * texcoord = new SbVec3f(dist[0]/tmp1, dist[1]/tmp2, dist[2]/tmp3);

  obj->texcoordlist.append(texcoord);
  return (void *) texcoord;
}

// Check if this cube is intersected by a faceset.
SbBool
Cvr3DTexSubCube::checkIntersectionFaceSet(const SbVec3f & cubeorigo,
                                          const SbVec3f * vertexlist,
                                          const int * numVertices,
                                          const unsigned int length,
                                          const SbMatrix & m)
{

  SbClip cubeclipper(this->subcube_clipperCB, this);
  this->origo = cubeorigo; // 'origo' is used by the 'renderBBox()'
  cubeclipper.reset();

  SbVec3f a;
  unsigned int idx = 0;
  for (unsigned int i=0;i<length;++i) {
    for (int j=0;j<numVertices[i];++j) {
      m.multVecMatrix(vertexlist[idx++], a);
      cubeclipper.addVertex(a);
    }
    this->clipPolygonAgainstCube(cubeclipper, cubeorigo);
    cubeclipper.reset();
  }

  return TRUE;

}

// Check if this cube is intersected by a triangle strip set.
SbBool
Cvr3DTexSubCube::checkIntersectionTriangleStripSet(const SbVec3f & cubeorigo,
                                                   const SbVec3f * vertexlist,
                                                   const int * numVertices,
                                                   const unsigned int length,
                                                   const SbMatrix & m)
{

  SbClip cubeclipper(this->subcube_clipperCB, this);
  this->origo = cubeorigo; // 'origo' is used by the 'renderBBox()'
  cubeclipper.reset();

  SbVec3f a;
  unsigned int idx = 0;
  for (unsigned int i=0;i<length;++i) {

    int counter = 0;
    for (int j=0;j<numVertices[i];++j) {

      m.multVecMatrix(vertexlist[idx++], a);
      cubeclipper.addVertex(a);
      if (counter == 2) {
        this->clipPolygonAgainstCube(cubeclipper, cubeorigo);
        cubeclipper.reset();

        if (j == (numVertices[i] - 1)) break; // Strip finished.

        counter = 0;
        j -= 2;
        idx -= 2;
      }
      else counter++;

    }
  }

  return TRUE;

}

// Check if this cube is intersected by an indexed triangle strip set.
SbBool
Cvr3DTexSubCube::checkIntersectionIndexedTriangleStripSet(const SbVec3f & cubeorigo,
                                                          const SbVec3f * vertexlist,
                                                          const int * indices,
                                                          const unsigned int numindices,
                                                          const SbMatrix & m)
{

  SbClip cubeclipper(this->subcube_clipperCB, this);
  this->origo = cubeorigo; // 'origo' is used by the 'renderBBox()'
  cubeclipper.reset();

  SbVec3f a;
  int counter = 0;
  for (unsigned int i=0;i<numindices;++i) {
    if (indices[i] == -1) {
      counter = 0;
      continue;
    }
    else {
      m.multVecMatrix(vertexlist[indices[i]], a);
      cubeclipper.addVertex(a);
      if (counter == 2) {
        this->clipPolygonAgainstCube(cubeclipper, cubeorigo);
        cubeclipper.reset();
        if ((i >= numindices) || indices[i+1] == -1) continue;
        counter = 0;
        i -= 2;
      }
      else counter++;
    }
  }

  return TRUE;

}


// Check if this cube is intersected by an indexed faceset.
SbBool
Cvr3DTexSubCube::checkIntersectionIndexedFaceSet(const SbVec3f & cubeorigo,
                                                 const SbVec3f * vertexlist,
                                                 const int * indices,
                                                 const unsigned int numindices,
                                                 const SbMatrix & m)
{

  SbClip cubeclipper(this->subcube_clipperCB, this);
  this->origo = cubeorigo; // 'origo' is used by the 'renderBBox()'
  cubeclipper.reset();

  SbVec3f a;
  for (unsigned int i=0;i<numindices;++i) {
    if (indices[i] != -1) {
      m.multVecMatrix(vertexlist[indices[i]], a);
      cubeclipper.addVertex(a);
    } else { // Index == -1. Clip polygon.
      this->clipPolygonAgainstCube(cubeclipper, cubeorigo);
      cubeclipper.reset();
    }
  }

  return this->clipPolygonAgainstCube(cubeclipper, cubeorigo);

}


// Check if this cube is intersected by the viewport aligned clip plane.
SbBool
Cvr3DTexSubCube::checkIntersectionSlice(const SbVec3f & cubeorigo,
                                        const SbViewVolume & viewvolume,
                                        const float viewdistance,
                                        const SbMatrix & m)
{

  SbClip cubeclipper(this->subcube_clipperCB, this);
  this->origo = cubeorigo; // 'origo' is used by the 'renderBBox()'
  cubeclipper.reset();

  // FIXME: Can we rewrite this to support viewport shells for proper
  // perspective? (20040227 handegar)
  // NOTE: If viewport shells is to be support, a separate method
  // must be added for the standard ObliqueSlice rendering.

  SbVec3f a, b, c, d;
  a = viewvolume.getPlanePoint(viewdistance, SbVec2f(-2.0f,  2.0f));
  b = viewvolume.getPlanePoint(viewdistance, SbVec2f( 2.0f,  2.0f));
  c = viewvolume.getPlanePoint(viewdistance, SbVec2f( 2.0f, -2.0f));
  d = viewvolume.getPlanePoint(viewdistance, SbVec2f(-2.0f, -2.0f));

  m.multVecMatrix(a, a);
  m.multVecMatrix(b, b);
  m.multVecMatrix(c, c);
  m.multVecMatrix(d, d);

  cubeclipper.addVertex(a);
  cubeclipper.addVertex(b);
  cubeclipper.addVertex(c);
  cubeclipper.addVertex(d);

  return this->clipPolygonAgainstCube(cubeclipper, cubeorigo);

}

// Internal method
SbBool
Cvr3DTexSubCube::clipPolygonAgainstCube(SbClip & cubeclipper, const SbVec3f & cubeorigo)
{
  /*
    NB!: the 'cubeclipper' object must have been initialized with a
    polygon *before* this function is called to have an effect.
  */

  // Clockwise direction for all planes
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
      slice.vertex.append(vert);

      SbVec3f * tmp = (SbVec3f *) cubeclipper.getVertexData(i);
      if (tmp == NULL) { tmp = (SbVec3f *) subcube_clipperCB(vert, NULL, vert, NULL, vert, this); }
      SbVec3f texcoord(tmp->getValue());
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

// *************************************************************************

void
Cvr3DTexSubCube::render(const SoGLRenderAction * action)
{
  // FIXME: A separate method for rendering sorted tris should be
  // made. This would be useful for the facesets. (20040630 handegar)

  if (this->volumeslices.getLength() == 0)
    return;

  // Texture binding/activation must happen before setting the
  // palette, or the previous palette will be used.
  this->textureobject->activateTexture(action);

  if (this->textureobject->isPaletted())  // Switch on palette rendering
    this->activateCLUT(action);

  if (CvrUtil::dontModulateTextures()) // Is texture mod. disabled by an envvar?
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

  // FIXME: Maybe we should build a vertex array instead of making
  // glVertex3f calls. Would probably give a performance
  // boost. (20040312 handegar)

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

  if (this->textureobject->isPaletted()) // Switch OFF palette rendering
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

// *************************************************************************

// FIXME: these definitely look like an indication of bad design, in
// client code. 20040716 mortene.

float
Cvr3DTexSubCube::getDistanceFromCamera(void) const
{
  return this->distancefromcamera;
}

void
Cvr3DTexSubCube::setDistanceFromCamera(float dist)
{
  this->distancefromcamera = dist;
}

// *************************************************************************
