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

#include <VolumeViz/render/3D/Cvr3DTexCube.h>

#include <limits.h>
#include <string.h>

#include <Inventor/C/glue/gl.h>
#include <Inventor/C/tidbits.h>
#include <Inventor/SbLinear.h>
#include <Inventor/SbViewVolume.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoProjectionMatrixElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/elements/SoViewingMatrixElement.h>
#include <Inventor/errors/SoDebugError.h>

#include <VolumeViz/elements/CvrPageSizeElement.h>
#include <VolumeViz/elements/CvrVoxelBlockElement.h>
#include <VolumeViz/elements/SoTransferFunctionElement.h>
#include <VolumeViz/misc/CvrCLUT.h>
#include <VolumeViz/misc/CvrUtil.h>
#include <VolumeViz/nodes/SoTransferFunction.h>
#include <VolumeViz/nodes/SoVolumeData.h>
#include <VolumeViz/render/common/Cvr3DPaletteTexture.h>
#include <VolumeViz/render/common/Cvr3DRGBATexture.h>
#include <VolumeViz/render/3D/Cvr3DTexSubCube.h>

// *************************************************************************

class Cvr3DTexSubCubeItem {
public:
  Cvr3DTexSubCubeItem(Cvr3DTexSubCube * p) { this->cube = p; }
  Cvr3DTexSubCube * cube;
  uint32_t volumedataid;
  SbVec3f center;
  SbBool invisible;
};

// *************************************************************************

Cvr3DTexCube::Cvr3DTexCube(const SoGLRenderAction * action)
{
  this->clut = NULL;
  this->subcubes = NULL;

  SoState * state = action->getState();

  this->subcubesize =
    Cvr3DTexCube::clampSubCubeSize(CvrPageSizeElement::get(state));

  if (CvrUtil::doDebugging()) {
    SoDebugError::postInfo("Cvr3DTexCube::Cvr3DTexCube",
                           "subcubedimensions==<%d, %d, %d>",
                           this->subcubesize[0],
                           this->subcubesize[1],
                           this->subcubesize[2]);
  }

  const CvrVoxelBlockElement * vbelem = CvrVoxelBlockElement::getInstance(state);
  const SbVec3s & dim = vbelem->getVoxelCubeDimensions();

  assert(dim[0] > 0);
  assert(dim[1] > 0);
  assert(dim[2] > 0);

  this->dimensions = dim;
  this->origo = SbVec3f(-((float) dim[0]) / 2.0f, -((float) dim[1]) / 2.0f, -((float) dim[2]) / 2.0f);

  this->nrcolumns = (this->dimensions[0] + this->subcubesize[0] - 1) / this->subcubesize[0];
  this->nrrows = (this->dimensions[1] + this->subcubesize[1] - 1) / this->subcubesize[1];
  this->nrdepths = (this->dimensions[2] + this->subcubesize[2] - 1) / this->subcubesize[2];

  this->rendersubcubeoutline = FALSE;
  const char * framesenvstr = coin_getenv("CVR_SUBCUBE_FRAMES");
  if (framesenvstr) { this->rendersubcubeoutline = atoi(framesenvstr) > 0 ? TRUE : FALSE; }

  this->abortfunc = NULL;
  this->abortfuncdata = NULL;
}

Cvr3DTexCube::~Cvr3DTexCube()
{
  this->releaseAllSubCubes();
  if (this->clut) { this->clut->unref(); }
}


void
Cvr3DTexCube::setAbortCallback(SoVolumeRenderAbortCB * func, void * userdata)
{
  this->abortfunc = func;
  this->abortfuncdata = userdata;
}

/*!
  Release resources used by a page in the slice.
*/
void
Cvr3DTexCube::releaseSubCube(const int row, const int col, const int depths)
{
  const int idx = this->calcSubCubeIdx(row, col, depths);
  Cvr3DTexSubCubeItem * p = this->subcubes[idx];
  this->subcubes[idx] = NULL;
  delete p->cube;
  delete p;
}

void
Cvr3DTexCube::releaseAllSubCubes(void)
{

  if (this->subcubes == NULL) return;

  for (int row = 0; row < this->nrrows; row++) {
    for (int col = 0; col < this->nrcolumns; col++) {
      for (int depth = 0; depth < this->nrdepths; depth++) {
        this->releaseSubCube(row, col, depth);
      }
    }
  }

  delete[] this->subcubes;
  this->subcubes = NULL;

}

static int
subcube_qsort_compare(const void * element1, const void * element2)
{
  Cvr3DTexSubCubeItem ** sc1 = (Cvr3DTexSubCubeItem **) element1;
  Cvr3DTexSubCubeItem ** sc2 = (Cvr3DTexSubCubeItem **) element2;

  if ((*sc1)->cube->getDistanceFromCamera() >
      (*sc2)->cube->getDistanceFromCamera()) return -1;
  else return 1;

}

SbVec3s
Cvr3DTexCube::clampSubCubeSize(const SbVec3s & size)
{
  // FIXME: this doesn't guarantee that we can actually use a texture
  // of this size, should instead use Coin's
  // cc_glglue_is_texture_size_legal() (at least in combination with
  // the subcubesize found here). 20040709 mortene.
  //
  // UPDATE: the above Coin cc_glglue function was introduced with
  // Coin 2.3, so we can't use this without first separating out the
  // gl-wrapper, as planned. 20040714 mortene.

  GLint maxsize;
  glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &maxsize);
  if (CvrUtil::doDebugging()) {
    SoDebugError::postInfo("Cvr3DTexCube::clampSubCubeSize",
                           "GL_MAX_3D_TEXTURE_SIZE==%d", maxsize);
  }

  const char * envstr = coin_getenv("CVR_FORCE_SUBCUBE_SIZE");
  if (envstr) {
    short forcedsubcubesize = atoi(envstr);
    assert(forcedsubcubesize > 0);
    assert(forcedsubcubesize <= maxsize && "subcube size must be <= than max 3D texture size");
    assert(coin_is_power_of_two(forcedsubcubesize) && "subcube size must be power of two");
    return SbVec3s(forcedsubcubesize, forcedsubcubesize, forcedsubcubesize);
  }

  // FIXME: My GeforceFX 5600 card sometime fails when asking for 512 as
  // cube size even if it is supposed to handle it. (20040302 handegar)
  //maxsize = SbMin(256, maxsize);

  assert((maxsize < SHRT_MAX) && "unsafe cast");
  const short smax = (short)maxsize;
  return SbVec3s(SbMin(size[0], smax), SbMin(size[1], smax), SbMin(size[2], smax));
}


// Called by all the 'render*()' methods after the intersection test.
void
Cvr3DTexCube::renderResult(const SoGLRenderAction * action,
                           SbList <Cvr3DTexSubCubeItem *> subcubelist)
{
  // Render all subcubes.
  for (int i=0;i<subcubelist.getLength();++i) {
    subcubelist[i]->cube->render(action);
  }
  // Draw lines around each subcube if requested by the 'CVR_SUBCUBE_FRAMES' envvar.
  if (this->rendersubcubeoutline) {
    for (int i=0;i<subcubelist.getLength();++i)
      subcubelist[i]->cube->renderBBox(action, i);
  }
  subcubelist.truncate(0);
}


// Renders arbitrary positioned quad, textured for the cube (slice)
// represented by this object. Loads all the cubes needed.
void
Cvr3DTexCube::render(const SoGLRenderAction * action,
                     const unsigned int numslices)
{
  const cc_glglue * glglue = cc_glglue_instance(action->getCacheContext());

  SoState * state = action->getState();

  SbVec3f subcubewidth = SbVec3f(this->subcubesize[0], 0, 0);
  SbVec3f subcubeheight = SbVec3f(0, this->subcubesize[1], 0);
  SbVec3f subcubedepth = SbVec3f(0, 0, this->subcubesize[2]);

  SbViewVolume viewvolume = SoViewVolumeElement::get(action->getState());
  SbViewVolume viewvolumeinv = viewvolume;
  viewvolumeinv.transform(SoModelMatrixElement::get(state).inverse());

  SbBox3f bbox(this->origo, this->origo +
               SbVec3f(this->dimensions[0],
                       this->dimensions[1],
                       this->dimensions[2]));
  bbox.transform(SoModelMatrixElement::get(state));
  float dx, dy, dz;
  bbox.getSize(dx, dy, dz);
  const float bboxradius = SbVec3f(dx, dy, dz).length() * 0.5f;

  const SbPlane camplane = viewvolume.getPlane(0.0f);
  const SbVec3f bboxcenter = bbox.getCenter();
  const float neardistance = SbAbs(camplane.getDistance(bboxcenter)) + bboxradius;
  const float fardistance = SbAbs(camplane.getDistance(bboxcenter)) - bboxradius;
  const float distancedelta = (fardistance - neardistance) / numslices;
  const SbMatrix mat = SoModelMatrixElement::get(state).inverse();

  SbList <Cvr3DTexSubCubeItem *> subcubelist;

  for (int rowidx = 0; rowidx < this->nrrows; rowidx++) {
    for (int colidx = 0; colidx < this->nrcolumns; colidx++) {
      for (int depthidx = 0; depthidx < this->nrdepths; depthidx++) {

        Cvr3DTexSubCube * cube = NULL;
        Cvr3DTexSubCubeItem * cubeitem = this->getSubCube(state, colidx, rowidx, depthidx);
        const SbVec3f subcubeorigo = this->origo +
          subcubewidth*colidx + subcubeheight*rowidx + subcubedepth*depthidx;

        if (cubeitem == NULL) { 
          cubeitem = this->buildSubCube(action, subcubeorigo, colidx, rowidx, depthidx); 
        }
        assert(cubeitem != NULL);

        if (cubeitem->invisible) continue;
        assert(cubeitem->cube != NULL);
     
        SbBox3f subbbox(subcubeorigo, subcubeorigo + subcubeheight + subcubewidth + subcubedepth);

        const float cubecameradist = (viewvolumeinv.getProjectionPoint() -
                                      subbbox.getCenter()).length();
        cubeitem->cube->setDistanceFromCamera(cubecameradist);

        subbbox.transform(SoModelMatrixElement::get(state));
        float sdx, sdy, sdz;
        subbbox.getSize(sdx, sdy, sdz);
        const float cuberadius = SbVec3f(sdx, sdy, sdz).length() * 0.5f;
        const float cubecenterdist = SbAbs(camplane.getDistance(subbbox.getCenter()));

        for (unsigned int i=0; i<numslices; ++i) {

          if (this->abortfunc != NULL) { // Check user-callback status.
            SoVolumeRender::AbortCode abortcode = this->abortfunc(numslices, (numslices - i), 
                                                                  this->abortfuncdata);
            if (abortcode == SoVolumeRender::ABORT) break;
            else if (abortcode == SoVolumeRender::SKIP) continue;
          }

          const float dist = fardistance - i*distancedelta;
          if (dist > (cubecenterdist+cuberadius)) break; // We have passed the cube.
          else if (dist < (cubecenterdist-cuberadius)) continue; // We haven't reached the cube yet.

          cubeitem->cube->intersectSlice(viewvolume, dist, mat);
        }

        subcubelist.append(cubeitem);

      }
    }
  }

  // Sort rendering order of the subcubes depending on the distance to
  // the camera.
  qsort((void *) subcubelist.getArrayPtr(),
        subcubelist.getLength(),
        sizeof(Cvr3DTexSubCubeItem *),
        subcube_qsort_compare);

  this->renderResult(action, subcubelist);
}

// Renders *one* slice of the volume according to the specified
// plane. Loads all the subcubes needed.
void
Cvr3DTexCube::renderObliqueSlice(const SoGLRenderAction * action,
                                 const SbPlane plane)
{

  const cc_glglue * glglue = cc_glglue_instance(action->getCacheContext());

  SoState * state = action->getState();

  SbVec3f subcubewidth = SbVec3f(this->subcubesize[0], 0, 0);
  SbVec3f subcubeheight = SbVec3f(0, this->subcubesize[1], 0);
  SbVec3f subcubedepth = SbVec3f(0, 0, this->subcubesize[2]);

  const float dist = plane.getDistanceFromOrigin();
  SbVec3f z = plane.getNormal();
  SbVec3f y(z[2], -z[1], z[0]);
  SbVec3f x = y.cross(z);
  x.normalize();
  y = z.cross(x);
  y.normalize();

  SbMatrix m, t;
  m.makeIdentity();
  m[0][0] = x[0]; m[0][1] = x[1]; m[0][2] = x[2];
  m[1][0] = y[0]; m[1][1] = y[1]; m[1][2] = y[2];
  m[2][0] = z[0]; m[2][1] = z[1]; m[2][2] = z[2];

  t.setTranslate(plane.getNormal() * dist);
  m = m * t;

  SbViewVolume viewvolume;
  // FIXME: Is this the correct way to setup a viewvolume? The
  // important goal is that the object is completely inside the ortho
  // area. (20040628 handegar)
  const float viewvolumesize = (subcubewidth + subcubeheight + subcubedepth).length();
  viewvolume.ortho(-viewvolumesize, viewvolumesize, -viewvolumesize, viewvolumesize, 0.01, 100);
  viewvolume.transform(m);
  const SbMatrix mat = SoModelMatrixElement::get(state).inverse();

  SbList <Cvr3DTexSubCubeItem *> subcubelist;

  for (int rowidx = 0; rowidx < this->nrrows; rowidx++) {
    for (int colidx = 0; colidx < this->nrcolumns; colidx++) {
      for (int depthidx = 0; depthidx < this->nrdepths; depthidx++) {

        Cvr3DTexSubCube * cube = NULL;
        Cvr3DTexSubCubeItem * cubeitem = this->getSubCube(state, colidx, rowidx, depthidx);

        const SbVec3f subcubeorigo = this->origo +
          subcubewidth*colidx + subcubeheight*rowidx + subcubedepth*depthidx;

        if (cubeitem == NULL) { 
          cubeitem = this->buildSubCube(action, subcubeorigo, colidx, rowidx, depthidx); 
        }
        assert(cubeitem != NULL);

        if (cubeitem->invisible) continue;
        assert(cubeitem->cube != NULL);
      
        cubeitem->cube->intersectSlice(viewvolume, 0, mat);
        subcubelist.append(cubeitem);

      }
    }
  }

  this->renderResult(action, subcubelist);
}

// Renders a indexed faceset inside the volume. Loads all the subcubes needed.
void
Cvr3DTexCube::renderIndexedSet(const SoGLRenderAction * action,
                               const SbVec3f * vertexarray,
                               const int * indices,
                               const unsigned int numindices,
                               const enum IndexedSetType type)
{

  assert(vertexarray);
  assert(indices);

  const cc_glglue * glglue = cc_glglue_instance(action->getCacheContext());

  SoState * state = action->getState();

  SbVec3f subcubewidth = SbVec3f(this->subcubesize[0], 0, 0);
  SbVec3f subcubeheight = SbVec3f(0, this->subcubesize[1], 0);
  SbVec3f subcubedepth = SbVec3f(0, 0, this->subcubesize[2]);

  SbList <Cvr3DTexSubCubeItem *> subcubelist;
  const SbMatrix invmodelmatrix = SoModelMatrixElement::get(state).inverse();

  for (int rowidx = 0; rowidx < this->nrrows; rowidx++) {
    for (int colidx = 0; colidx < this->nrcolumns; colidx++) {
      for (int depthidx = 0; depthidx < this->nrdepths; depthidx++) {

        Cvr3DTexSubCube * cube = NULL;
        Cvr3DTexSubCubeItem * cubeitem = this->getSubCube(state, colidx, rowidx, depthidx);

        const SbVec3f subcubeorigo = this->origo +
          subcubewidth*colidx + subcubeheight*rowidx + subcubedepth*depthidx;
        
        if (cubeitem == NULL) { 
          cubeitem = this->buildSubCube(action, subcubeorigo, colidx, rowidx, depthidx); 
        }
        assert(cubeitem != NULL);

        if (cubeitem->invisible) continue;
        assert(cubeitem->cube != NULL);

        if (type == Cvr3DTexCube::INDEXEDFACE_SET) {
          cubeitem->cube->intersectIndexedFaceSet(vertexarray,
                                                  indices,
                                                  numindices,
                                                  invmodelmatrix);
        }
        else if (type == Cvr3DTexCube::INDEXEDTRIANGLESTRIP_SET) {
          cubeitem->cube->intersectIndexedTriangleStripSet(vertexarray,
                                                           indices,
                                                           numindices,
                                                           invmodelmatrix);
        }
        else assert(FALSE && "Unknown set type!");

        subcubelist.append(cubeitem);

      }
    }
  }

  this->renderResult(action, subcubelist);
}

// Renders a nonindexed faceset inside the volume. Loads all the subcubes needed.
void
Cvr3DTexCube::renderNonindexedSet(const SoGLRenderAction * action,
                                  const SbVec3f * vertexarray,
                                  const int * numVertices,
                                  const unsigned int listlength,
                                  const enum NonindexedSetType type)
{

  assert(vertexarray);
  assert(numVertices);

  const cc_glglue * glglue = cc_glglue_instance(action->getCacheContext());

  SoState * state = action->getState();

  SbVec3f subcubewidth = SbVec3f(this->subcubesize[0], 0, 0);
  SbVec3f subcubeheight = SbVec3f(0, this->subcubesize[1], 0);
  SbVec3f subcubedepth = SbVec3f(0, 0, this->subcubesize[2]);

  SbList <Cvr3DTexSubCubeItem *> subcubelist;
  const SbMatrix invmodelmatrix = SoModelMatrixElement::get(state).inverse();

  for (int rowidx = 0; rowidx < this->nrrows; rowidx++) {
    for (int colidx = 0; colidx < this->nrcolumns; colidx++) {
      for (int depthidx = 0; depthidx < this->nrdepths; depthidx++) {

        Cvr3DTexSubCube * cube = NULL;
        Cvr3DTexSubCubeItem * cubeitem = this->getSubCube(state, colidx, rowidx, depthidx);

        const SbVec3f subcubeorigo = this->origo +
          subcubewidth*colidx + subcubeheight*rowidx + subcubedepth*depthidx;

        if (cubeitem == NULL) { 
          cubeitem = this->buildSubCube(action, subcubeorigo, colidx, rowidx, depthidx); 
        }
        assert(cubeitem != NULL);

        if (cubeitem->invisible) continue;
        assert(cubeitem->cube != NULL);

        if (type == Cvr3DTexCube::FACE_SET) {
          cubeitem->cube->intersectFaceSet(vertexarray,
                                           numVertices,
                                           listlength,
                                           invmodelmatrix);
        }
        else if (type == Cvr3DTexCube::TRIANGLESTRIP_SET) {
          cubeitem->cube->intersectTriangleStripSet(vertexarray,
                                                    numVertices,
                                                    listlength,
                                                    invmodelmatrix);
        }
        else assert(FALSE && "Unknown set type!");

        subcubelist.append(cubeitem);

      }
    }
  }

  this->renderResult(action, subcubelist);
}




int
Cvr3DTexCube::calcSubCubeIdx(int row, int col, int depth) const
{
  assert((row >= 0) && (row < this->nrrows));
  assert((col >= 0) && (col < this->nrcolumns));
  assert((depth >= 0) && (depth < this->nrdepths));

  int idx = (col + (row * this->nrcolumns)) + (depth * this->nrcolumns * this->nrrows);

  assert(idx < (this->nrdepths * this->nrcolumns * this->nrrows));

  return idx;
}

// Builds a cube if it doesn't exist. Rebuilds it if it does exist.
Cvr3DTexSubCubeItem *
Cvr3DTexCube::buildSubCube(const SoGLRenderAction * action,
                           const SbVec3f & subcubeorigo,
                           int col, int row, int depth)
{
  // FIXME: optimalization idea; *crop* textures for 100%
  // transparency. 20021124 mortene.

  // FIXME: optimalization idea; detect 100% similar neighboring
  // cubes, and make cubes able to map to several "slice indices". Not
  // sure if this can be much of a gain -- but look into it. 20021124 mortene.

  assert((this->getSubCube(action->getState(), col, row, depth) == NULL) && "Subcube already created!");

  // First Cvr3DTexSubCube ever in this slice?
  if (this->subcubes == NULL) {
    if (CvrUtil::doDebugging()) {
      SoDebugError::postInfo("Cvr3DTexCube::buildSubCube",
                             "number of subcubes needed == %d (%d x %d x %d)",
                             this->nrrows * this->nrcolumns * this->nrdepths,
                             this->nrrows, this->nrcolumns, this->nrdepths);
    }

    this->subcubes = new Cvr3DTexSubCubeItem*[this->nrrows * this->nrcolumns * this->nrdepths];
    for (int i=0; i < this->nrrows; i++) {
      for (int j=0; j < this->nrcolumns; j++) {
        for (int k=0; k < this->nrdepths; k++) {
          const int idx = this->calcSubCubeIdx(i, j, k);
          this->subcubes[idx] = NULL;
        }
      }
    }
  }

  SbVec3s subcubemin, subcubemax;
  if (CvrUtil::useFlippedYAxis()) {
    // NOTE: Building subcubes 'upwards' so that the Y orientation
    // will be equal to the 2D slice rendering (the voxelchunks are
    // also flipped).
    subcubemin = SbVec3s(col * this->subcubesize[0],
                         this->dimensions[1] - (row + 1) * this->subcubesize[1],
                         depth * this->subcubesize[2]);
    subcubemax = SbVec3s((col + 1) * this->subcubesize[0],
                         this->dimensions[1] - row * this->subcubesize[1],
                         (depth + 1) * this->subcubesize[2]);
  }
  else {
    subcubemin = SbVec3s(col * this->subcubesize[0],
                         row * this->subcubesize[1],
                         depth * this->subcubesize[2]);
    subcubemax = SbVec3s((col + 1) * this->subcubesize[0],
                         (row + 1) * this->subcubesize[1],
                         (depth + 1) * this->subcubesize[2]);
  }

  // Crop subcube size
  subcubemax[0] = SbMin(subcubemax[0], this->dimensions[0]);
  subcubemax[1] = SbMin(subcubemax[1], this->dimensions[1]);
  subcubemax[2] = SbMin(subcubemax[2], this->dimensions[2]);

  subcubemin[1] = SbMax(subcubemin[1], (short) 0);

#if CVR_DEBUG && 0 // debug
  SoDebugError::postInfo("Cvr3DTexCube::buildSubCube",
                         "subcubemin=[%d, %d, %d] subcubemax=[%d, %d, %d]",
                         subcubemin[0], subcubemin[1], subcubemin[2],
                         subcubemax[0], subcubemax[1], subcubemax[2]);
#endif // debug
  SbBox3s subcubecut = SbBox3s(subcubemin, subcubemax);

  SoState * state = action->getState();
  const CvrVoxelBlockElement * vbelem = CvrVoxelBlockElement::getInstance(state);
  assert(vbelem != NULL);

  const SbVec3s texsize(subcubemax - subcubemin);
  const CvrTextureObject * texobj = CvrTextureObject::create(action, texsize, subcubecut);
  // if NULL is returned, it means all voxels are fully transparent

  Cvr3DTexSubCube * cube = NULL;
  if (texobj) {
    short dx, dy, dz;
    subcubecut.getSize(dx, dy, dz);
    const SbVec3f cubesize(dx, dy, dz);
    cube = new Cvr3DTexSubCube(action, texobj, subcubeorigo, cubesize, texsize);
    cube->setPalette(this->clut);
  }

  Cvr3DTexSubCubeItem * pitem = new Cvr3DTexSubCubeItem(cube);
  pitem->volumedataid = vbelem->getNodeId();
  pitem->invisible = (texobj == NULL) ? TRUE : FALSE;

  const int idx = this->calcSubCubeIdx(row, col, depth);
  this->subcubes[idx] = pitem;

  return pitem;
}

// *******************************************************************

void
Cvr3DTexCube::setPalette(const CvrCLUT * c)
{

  if (this->clut) { this->clut->unref(); }
  this->clut = c;
  this->clut->ref();

  if (this->subcubes == NULL) return;

  // Change palette for all subcubes.
  for (int col=0; col < this->nrcolumns; col++) {
    for (int row=0; row < this->nrrows; row++) {
      for (int depth = 0; depth < this->nrdepths; depth++) {
        const int idx = this->calcSubCubeIdx(row, col, depth);
        Cvr3DTexSubCubeItem * subc = this->subcubes[idx];
        // FIXME: as far as I can tell from the code, the extra
        // "subp->cube != NULL" check here should be superfluous, but I
        // get a NULL-ptr crash if I leave it out when using RGBA
        // textures and changing the palette. (Reproducible on
        // ASK.trh.sim.no.) Investigate further. 20030325 mortene.
        if (subc && subc->cube) {
          if (subc->cube->isPaletted()) { subc->cube->setPalette(this->clut); }
          else { this->releaseSubCube(row, col, depth); }
        }
      }
    }
  }
}

// *******************************************************************

const CvrCLUT *
Cvr3DTexCube::getPalette(void) const
{
  return this->clut;
}

// *******************************************************************

Cvr3DTexSubCubeItem *
Cvr3DTexCube::getSubCube(SoState * state, int col, int row, int depth)
{
  
  if (this->subcubes == NULL) return NULL;

  assert((col >= 0) && (col < this->nrcolumns));
  assert((row >= 0) && (row < this->nrrows));
  assert((depth >= 0) && (depth < this->nrdepths));

  const int idx = this->calcSubCubeIdx(row, col, depth);
  Cvr3DTexSubCubeItem * subp = this->subcubes[idx];

  if (subp) {
    const CvrVoxelBlockElement * vbelem = CvrVoxelBlockElement::getInstance(state);
    assert(vbelem != NULL);
    uint32_t volumedataid = vbelem->getNodeId();

    if (subp->volumedataid != volumedataid) {
      // FIXME: it could perhaps be a decent optimalization to store a
      // checksum value along with the subcube, to use for comparison
      // to see if this subcube really changed. 20030220 mortene.
      this->releaseSubCube(row, col, depth);
      return NULL;
    }
  }
  
  return subp;
}
