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
 *  Systems in Motion, Postboks 1283, Pirsenteret, 7462 Trondheim, NORWAY.
 *  <URL:http://www.sim.no/>.
 *
\**************************************************************************/

#include <VolumeViz/render/3D/Cvr3DTexSubCube.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include <Inventor/C/glue/gl.h>
#include <Inventor/C/tidbits.h>
#include <Inventor/SbMatrix.h>
#include <Inventor/SbViewVolume.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/nodes/SoDrawStyle.h>
#include <Inventor/misc/SoState.h>

#include <VolumeViz/elements/CvrLightingElement.h>
#include <VolumeViz/misc/CvrCLUT.h>
#include <VolumeViz/misc/CvrUtil.h>
#include <VolumeViz/render/common/Cvr3DPaletteTexture.h>


// *************************************************************************

/*! \a cubeorigo is the "lower left" position of this subcube in the
    local coordinate system of the full volume. E.g. for the
    LOBSTER.VOL example model of dimensions <320, 320, 34>, the first
    subcube would have its parameter cubeorigo==<-160, -160, -17>.

    \a cubesize is the voxel dimensions of the sub-cube.
*/
Cvr3DTexSubCube::Cvr3DTexSubCube(const SoGLRenderAction * action,
                                 const CvrTextureObject * texobj,
                                 const SbVec3f & cubeorigo,
                                 const SbVec3s & cubesize)
{
  this->clut = NULL;

  assert(cubesize[0] >= 0);
  assert(cubesize[1] >= 0);
  assert(cubesize[2] >= 0);

  this->dimensions = cubesize;

  if (texobj->getTypeId() == Cvr3DPaletteTexture::getClassTypeId()) {
    this->clut = ((CvrPaletteTexture *)texobj)->getCLUT();
    this->clut->ref();
  }

  this->textureobject = texobj;
  this->textureobject->ref();

  // Calculate clipplanes
  this->clipplanes[0] = SbPlane(cubeorigo + SbVec3f(0.0f, this->dimensions[1], 0.0f),
                                cubeorigo,
                                cubeorigo + SbVec3f(this->dimensions[0], 0.0f, 0.0f));
  this->clipplanes[1] = SbPlane(cubeorigo + SbVec3f(this->dimensions[0], 0.0f, this->dimensions[2]),
                                cubeorigo + SbVec3f(0.0f, 0.0f, this->dimensions[2]),
                                cubeorigo + SbVec3f(0.0f, this->dimensions[1], this->dimensions[2]));
  this->clipplanes[2] = SbPlane(cubeorigo + SbVec3f(this->dimensions[0], 0.0f, 0.0f),
                                cubeorigo,
                                cubeorigo + SbVec3f(0.0f, 0.0f,  this->dimensions[2]));
  this->clipplanes[3] = SbPlane(cubeorigo + SbVec3f(0.0f, this->dimensions[1], this->dimensions[2]),
                                cubeorigo + SbVec3f(0.0f, this->dimensions[1], 0.0f),
                                cubeorigo + SbVec3f(this->dimensions[0], this->dimensions[1], 0.0f));
  this->clipplanes[4] = SbPlane(cubeorigo + SbVec3f(this->dimensions[0], this->dimensions[1], 0.0f),
                                cubeorigo + SbVec3f(this->dimensions[0], 0.0f, 0.0f),
                                cubeorigo + SbVec3f(this->dimensions[0], 0.0f, this->dimensions[2]));
  this->clipplanes[5] = SbPlane(cubeorigo + SbVec3f(0.0f, 0.0f, this->dimensions[2]),
                                cubeorigo,
                                cubeorigo + SbVec3f(0.0f, this->dimensions[1], 0.0f));
  
  this->origo = cubeorigo;

  this->volumesliceslength = 0;
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
}


// *************************************************************************

// FIXME: almost identical with 2DTexSubPage's ditto, should be
// possible to share. 20040719 mortene.

void
Cvr3DTexSubCube::activateCLUT(const SoGLRenderAction * action)
{
  assert(this->clut != NULL);

  const CvrLightingElement * lightelem = CvrLightingElement::getInstance(action->getState());
  assert(lightelem != NULL);
  const SbBool lighting = lightelem->useLighting(action->getState());
  if (lighting) {
    this->clut->activate(action->getCacheContext(), CvrCLUT::TEXTURE3D_GRADIENT);

    SbVec3f lightDir;
    float lightIntensity;
    lightelem->get(action->getState(), lightDir, lightIntensity);
    lightDir.normalize();
    const cc_glglue * glue = cc_glglue_instance(action->getCacheContext());
    cc_glglue_glProgramLocalParameter4f(glue, GL_FRAGMENT_PROGRAM_ARB, 1,
                                      lightDir[0], lightDir[1], lightDir[2], lightIntensity);
  } else {
    // FIXME: should check if the same clut is already current
    this->clut->activate(action->getCacheContext(), CvrCLUT::TEXTURE3D);
  }

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

// Check if this cube is intersected by a faceset.
void
Cvr3DTexSubCube::intersectFaceSet(const SbVec3f * vertexlist,
                                  const int * numVertices,
                                  const unsigned int length,
                                  const SbMatrix & m)
{
  this->clippoly.reset();

  SbVec3f a;
  unsigned int idx = 0;
  for (unsigned int i=0;i<length;++i) {
    for (int j=0;j<numVertices[i];++j) {
      m.multVecMatrix(vertexlist[idx++], a);
      this->clippoly.addVertex(a);
    }
    this->clipPolygonAgainstCube();
  }
}


// Check if this cube is intersected by a triangle strip set.
void
Cvr3DTexSubCube::intersectTriangleStripSet(const SbVec3f * vertexlist,
                                           const int * numVertices,
                                           const unsigned int length,
                                           const SbMatrix & m)
{
  this->clippoly.reset();

  SbVec3f a;
  unsigned int idx = 0;
  for (unsigned int i=0;i<length;++i) {

    int counter = 0;
    for (int j=0;j<numVertices[i];++j) {

      m.multVecMatrix(vertexlist[idx++], a);
      this->clippoly.addVertex(a);
      if (counter == 2) {
        this->clipPolygonAgainstCube();
        this->clippoly.reset();

        if (j == (numVertices[i] - 1)) break; // Strip finished.

        counter = 0;
        j -= 2;
        idx -= 2;
      }
      else counter++;

    }
  }
}


// Check if this cube is intersected by an indexed triangle strip set.
void
Cvr3DTexSubCube::intersectIndexedTriangleStripSet(const SbVec3f * vertexlist, 
                                                  const int * indices,
                                                  const unsigned int numindices,
                                                  const SbMatrix & m)
{
  this->clippoly.reset();

  SbVec3f a;
  int counter = 0;
  for (unsigned int i=0;i<numindices;++i) {
    if (indices[i] == -1) {
      counter = 0;
      continue;
    }
    else {
      m.multVecMatrix(vertexlist[indices[i]], a);
      this->clippoly.addVertex(a);
      if (counter == 2) {
        this->clipPolygonAgainstCube();
        this->clippoly.reset();
        if ((i >= numindices) || indices[i+1] == -1) continue;
        counter = 0;
        i -= 2;
      }
      else counter++;
    }
  }

}


// Check if this cube is intersected by an indexed faceset.
void
Cvr3DTexSubCube::intersectIndexedFaceSet(const SbVec3f * vertexlist,
                                         const int * indices,
                                         const unsigned int numindices,
                                         const SbMatrix & m)
{
  this->clippoly.reset();

  SbVec3f a;
  for (unsigned int i=0;i<numindices;++i) {
    if (indices[i] != -1) {
      m.multVecMatrix(vertexlist[indices[i]], a);
      this->clippoly.addVertex(a);
    }
    else { // Index == -1. Clip polygon.
      this->clipPolygonAgainstCube();
      this->clippoly.reset();
    }
  }

  this->clipPolygonAgainstCube();
}


// Check if this cube is intersected by the viewport aligned clip plane.
void
Cvr3DTexSubCube::intersectSlice(const SbVec3f * sliceplanecorners)
{
  this->clippoly.reset();
  for (unsigned int i=0; i < 4; i++) { this->clippoly.addVertex(sliceplanecorners[i]); }
  this->clipPolygonAgainstCube();
}


// Check if this cube is intersected by the viewport aligned clip plane.
void
Cvr3DTexSubCube::intersectSlice(const SbViewVolume & viewvolume,
                                const float viewdistance,
                                const SbMatrix & m)
{
  this->clippoly.reset();

  // FIXME: Can we rewrite this to support viewport shells for proper
  // perspective? (20040227 handegar)
  // NOTE: If viewport shells is to be supported, a separate method
  // must be added for the standard ObliqueSlice rendering.

  static const SbVec2f p1(-2.0f,  2.0f);
  static const SbVec2f p2( 2.0f,  2.0f);
  static const SbVec2f p3( 2.0f, -2.0f);
  static const SbVec2f p4(-2.0f, -2.0f);

  SbVec3f a = viewvolume.getPlanePoint(viewdistance, p1);
  SbVec3f b = viewvolume.getPlanePoint(viewdistance, p2);
  SbVec3f c = viewvolume.getPlanePoint(viewdistance, p3);
  SbVec3f d = viewvolume.getPlanePoint(viewdistance, p4);

  m.multVecMatrix(a, a);
  m.multVecMatrix(b, b);
  m.multVecMatrix(c, c);
  m.multVecMatrix(d, d);

  this->clippoly.addVertex(a);
  this->clippoly.addVertex(b);
  this->clippoly.addVertex(c);
  this->clippoly.addVertex(d);

  this->clipPolygonAgainstCube();
}


// Internal method
void
Cvr3DTexSubCube::clipPolygonAgainstCube(void)
{
  /*
    NB!: the 'this->clippoly' object must have been initialized with a
    polygon *before* this function is called to have an effect.
  */

  for (unsigned int j=0; j < 6; j++) {
    this->clippoly.clip(this->clipplanes[j]);
  }

  const unsigned int nrvertices = this->clippoly.getNumVertices();

  if (nrvertices >= 3) {
    const SbVec3s texdims = this->textureobject->getDimensions();
    SbVec3f vert;
    subcube_slice * slice = NULL;

    // We use our own tracking of the list-length, instead of doing
    // SbList::truncate(), so we can reuse the list elements (to avoid
    // unnecessary memory allocation).
    const unsigned int reallength = (unsigned int)this->volumeslices.getLength();
    assert(this->volumesliceslength <= reallength);
    if (this->volumesliceslength == reallength) {
      subcube_slice s;
      this->volumeslices.append(s);
    }

    slice = &this->volumeslices[this->volumesliceslength++];
    slice->vertex.truncate(0);
    slice->texcoord.truncate(0);

    // Due to the padding of subcubes which are not of size 2^n, we'll
    // have to cap the calculated texture coordinate with one voxel to
    // prevent OpenGL from interpolating "into the" padded data (only
    // visible when using GL_LINEAR and the voxelvalue zero has a
    // non-transparent color).
    // This resolves issue COINSUPPORT-1264.
    SbVec3s texdimsmodded = texdims;
    for (int i=0;i<3;++i) {
      if (this->dimensions[i] < texdims[i])
        texdimsmodded[i] += 1;
    }      
    
    for (unsigned int i=0; i < nrvertices; i++) {
      this->clippoly.getVertex(i, vert);
      slice->vertex.append(vert);
      
      const SbVec3f dist = vert - this->origo;
      const SbVec3f v(dist[0] / texdimsmodded[0], 
                      dist[1] / texdimsmodded[1], 
                      dist[2] / texdimsmodded[2]);

      slice->texcoord.append(v);
    }
  }
}


// *************************************************************************

void
Cvr3DTexSubCube::renderSlices(const SoGLRenderAction * action, SbBool wireframe)
{
  if (wireframe) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  }
  else {
    // Texture binding/activation must happen before setting the
    // palette, or the previous palette will be used.
    this->textureobject->activateTexture(action);
    if (this->textureobject->isPaletted()) { this->activateCLUT(action); }
  }

  if (CvrUtil::dontModulateTextures()) // Is texture mod. disabled by an envvar?
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

  // FIXME: Maybe we should build a vertex array instead of making
  // glVertex3f calls. Would probably give a performance
  // boost. (20040312 handegar)
  //
  // COMMENT 20040804 mortene: sounds unlikely to be a significant
  // bottleneck, IMHO.

  for (int i = this->volumesliceslength - 1; i >= 0; --i) {
    struct subcube_slice & slice = this->volumeslices[i];

    glBegin(GL_TRIANGLE_FAN);
    for (int j = 0; j < slice.vertex.getLength() ; ++j) {
      glTexCoord3fv(slice.texcoord[j].getValue());
      glVertex3fv(slice.vertex[j].getValue());
    }
    glEnd();

    slice.vertex.truncate(0);
    slice.texcoord.truncate(0);

    assert(glGetError() == GL_NO_ERROR);
  }

  // We use our own tracking of the list-length, instead of doing
  // SbList::truncate(), so we can reuse the list elements (to avoid
  // unnecessary memory allocation).
  this->volumesliceslength = 0;

  if (!wireframe && this->textureobject->isPaletted()) {
    this->deactivateCLUT(action);
  }
}


void
Cvr3DTexSubCube::render(const SoGLRenderAction * action)
{
  // FIXME: A separate method for rendering sorted tris should be
  // made. This would be useful for the facesets. (20040630 handegar)
  //
  // COMMENT 20040804 mortene: I don't understand this FIXME, please
  // elaborate.

  if (CvrUtil::doDebugging() && FALSE) {
    SoDebugError::postInfo("Cvr3DTexSubCube::render",
                           "slices==%d",
                           this->volumesliceslength);
  }

  // This can e.g. happen when some of the sub-cubes are not within
  // the view volume:
  if (this->volumesliceslength == 0) { return; }

  // 0: as usual, 1: added box wireframes, 2: only slice wireframes
  unsigned int renderstyle = CvrUtil::debugRenderStyle();

  // Shall we draw the oblique slice as lines/wireframe?
  SoDrawStyleElement::Style drawstyle = SoDrawStyleElement::get(action->getState());
  if (drawstyle == SoDrawStyleElement::LINES) renderstyle = 2;

  this->renderSlices(action, renderstyle == 2);
  if (renderstyle == 1) { this->renderBBox(); }
}


// For debugging purposes
void
Cvr3DTexSubCube::renderBBox(void) const
{
  glPushAttrib(GL_CULL_FACE | GL_DEPTH_TEST | GL_TEXTURE_2D | GL_TEXTURE_3D | GL_BLEND);

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

  glPopAttrib();
}

// *************************************************************************
