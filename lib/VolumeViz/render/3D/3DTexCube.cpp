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
#include <VolumeViz/render/common/Cvr3DPaletteTexture.h>
#include <VolumeViz/render/common/Cvr3DRGBATexture.h>
#include <VolumeViz/render/3D/Cvr3DTexSubCube.h>

// *************************************************************************

class Cvr3DTexSubCubeItem {
public:
  Cvr3DTexSubCubeItem(Cvr3DTexSubCube * p) { this->cube = p; }
  Cvr3DTexSubCube * cube;
  uint32_t volumedataid; // FIXME: seems bogus to store this here, as
                         // all sub-cubes will have the same
                         // value. 20040916 mortene.

  SbBool invisible; // If this flag is set, the value of "cube" should
                    // be NULL.

  // Distance from camera projection point (in the near plane) to the
  // sub-cube's center. Used for comparison with other sub-cubes when
  // qsort'ing by depth vs camera position.
  float distancefromcamera;

  float boundingsphereradius;
  float cameraplane2cubecenter;
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
Cvr3DTexCube::releaseSubCube(const unsigned int row, const unsigned int col, const unsigned int depths)
{
  const int idx = this->calcSubCubeIdx(row, col, depths);
  Cvr3DTexSubCubeItem * p = this->subcubes[idx];
  if (p) {
    this->subcubes[idx] = NULL;
    delete p->cube;
    delete p;
  }
}


void
Cvr3DTexCube::releaseAllSubCubes(void)
{
  if (this->subcubes == NULL) return;

  for (unsigned int row = 0; row < this->nrrows; row++) {
    for (unsigned int col = 0; col < this->nrcolumns; col++) {
      for (unsigned int depth = 0; depth < this->nrdepths; depth++) {
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

  return ((*sc1)->distancefromcamera > (*sc2)->distancefromcamera) ? -1 : 1;
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

  // FIXME: this design is a bit bogus. Consider this: the size can be
  // set in one GL context, but the tex-cube can later be attempted
  // used in another GL context, with a smaller max size. Not sure how
  // to fix this yet. 20041221 mortene.

  GLint maxsize = -1;
  glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &maxsize);

  // This has been reported by an external developer to hit on an ATI
  // OpenGL driver on a Linux system. As 3D texture based rendering
  // otherwise seemed to work just fine, we simply warn, correct the
  // problem, and go on.
  if (maxsize == -1) {
    static const char CVR_IGNORE_ATI_QUERY_BUG[] = "CVR_IGNORE_ATI_QUERY_BUG";
    const char * env = coin_getenv(CVR_IGNORE_ATI_QUERY_BUG);

    static SbBool first = TRUE;
    if (first && (env == NULL)) {
      SoDebugError::postWarning("Cvr3DTexCube::clampSubCubeSize",
                                "Obscure bug found with your OpenGL driver. "
                                "If you are employed by Systems in Motion, "
                                "report this occurrence to <mortene@sim.no> "
                                "for further debugging. Otherwise, you can "
                                "safely ignore this warning. (Set the "
                                "environment variable '%s' on the system to "
                                "not get this notification again.)",
                                CVR_IGNORE_ATI_QUERY_BUG);
    }
    first = FALSE;
    maxsize = 128; // this should be safe
  }

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
                           SbList <Cvr3DTexSubCubeItem *> & subcubelist)
{
  // Render all subcubes.
  for (int i=0;i<subcubelist.getLength();++i) {
    subcubelist[i]->cube->render(action); 
  }
  subcubelist.truncate(0);
}


// Renders arbitrary positioned quad, textured for the cube (slice)
// represented by this object. Loads all the cubes needed.
void
Cvr3DTexCube::render(const SoGLRenderAction * action,
                     unsigned int numslices)
{
  // For debugging purposes, make it possible to override the number
  // of slices to render with an envvar:
  static unsigned int forcednumslices = UINT_MAX;
  if (forcednumslices == UINT_MAX) {
    const char * env = coin_getenv("CVR_DEBUG_MAX_SLICES");
    int num = 0;
    if (env) { num = atoi(env); }
    assert(num >= 0);
    forcednumslices = num;
  }
  if (forcednumslices != 0) { numslices = forcednumslices; }


  const cc_glglue * glglue = cc_glglue_instance(action->getCacheContext());

  SoState * state = action->getState();

  const SbVec3f subcubewidth(this->subcubesize[0], 0, 0);
  const SbVec3f subcubeheight(0, this->subcubesize[1], 0);
  const SbVec3f subcubedepth(0, 0, this->subcubesize[2]);

  const SbViewVolume & viewvolume = SoViewVolumeElement::get(state);
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
  const SbPlane invcamplane = viewvolumeinv.getPlane(0.0f);
  const SbVec3f bboxcenter = bbox.getCenter();
  const float neardistance = SbAbs(camplane.getDistance(bboxcenter)) - bboxradius;
  const float fardistance = SbAbs(camplane.getDistance(bboxcenter)) + bboxradius;
  const float distancedelta = (fardistance - neardistance) / numslices;
  const SbMatrix mat = SoModelMatrixElement::get(state).inverse();

  SbList <Cvr3DTexSubCubeItem *> subcubelist;

  unsigned int startrow = 0, endrow = this->nrrows - 1;
  unsigned int startcolumn = 0, endcolumn = this->nrcolumns - 1;
  unsigned int startdepth = 0, enddepth = this->nrdepths - 1;

  // debug: this block of code detects and forces rendering of just a
  // single sub-cube if that is wanted
  static unsigned int forcerow = UINT_MAX, forcecolumn, forcedepth;
  if (forcerow == UINT_MAX) {
    const char * str = coin_getenv("CVR_DEBUG_FORCE_SUBCUBE");
    if (str) {
      const int nr =
        sscanf(str, "%u,%u,%u", &forcerow, &forcecolumn, &forcedepth);
      assert(nr == 3 && "Invalid CVR_DEBUG_FORCE_SUBCUBE param. Expected '=X,Y,Z'");
      SoDebugError::postInfo("Cvr3DTexCube::render",
                             "debug: forced rendering of sub-cube "
                             "<%u, %u, %u> only",
                             forcerow, forcecolumn, forcedepth);
      assert(forcerow < this->nrrows);
      assert(forcecolumn < this->nrcolumns);
      assert(forcedepth < this->nrdepths);
    }
    else {
      forcerow--;
    }
  }
  if (forcerow != UINT_MAX - 1) {
    startrow = endrow = forcerow;
    startcolumn = endcolumn = forcecolumn;
    startdepth = enddepth = forcedepth;
  }
  // debug end

  for (unsigned int rowidx = startrow; rowidx <= endrow; rowidx++) {
    for (unsigned int colidx = startcolumn; colidx <= endcolumn; colidx++) {
      for (unsigned int depthidx = startdepth; depthidx <= enddepth; depthidx++) {

        Cvr3DTexSubCubeItem * cubeitem = this->getSubCube(state, colidx, rowidx, depthidx);

        const SbVec3f subcubeorigo =
          this->origo +
          subcubewidth * (float)colidx +
          subcubeheight * (float)rowidx +
          subcubedepth * (float)depthidx;

        if (cubeitem == NULL) { 
          cubeitem = this->buildSubCube(action, subcubeorigo, colidx, rowidx, depthidx); 
        }
        assert(cubeitem != NULL);

        if (cubeitem->invisible) continue;
        assert(cubeitem->cube != NULL);

        subcubelist.append(cubeitem);

        SbBox3f subbbox(subcubeorigo, subcubeorigo + subcubeheight + subcubewidth + subcubedepth);
        float dist = -invcamplane.getDistance(subbbox.getCenter());

        //subbbox.transform(SoModelMatrixElement::get(state));
        
        if (viewvolumeinv.getProjectionType() == SbViewVolume::ORTHOGRAPHIC) {
          cubeitem->distancefromcamera = dist;
        }
        else {
          SbVec3f p = viewvolumeinv.getProjectionPoint();

          cubeitem->distancefromcamera = 
            (float) sqrt((viewvolumeinv.getProjectionPoint() - subbbox.getCenter()).length());

          if (dist < 0) {
            cubeitem->distancefromcamera = -cubeitem->distancefromcamera;
          }
        }

        subbbox.transform(SoModelMatrixElement::get(state));
        float sdx, sdy, sdz;
        subbbox.getSize(sdx, sdy, sdz);
        cubeitem->boundingsphereradius = SbVec3f(sdx, sdy, sdz).length() * 0.5f;
        cubeitem->cameraplane2cubecenter = SbAbs(camplane.getDistance(subbbox.getCenter()));

#if 0 // debug
        printf("cubeitem %u,%u,%u, distancefromcamera==%f, boundingsphereradius==%f, "
               "cameraplane2cubecenter==%f\n",
               rowidx, colidx, depthidx,
               cubeitem->distancefromcamera,
               cubeitem->boundingsphereradius,
               cubeitem->cameraplane2cubecenter);
#endif // debug
      }
    }
  }

  // FIXME: Can we rewrite this to support viewport shells for proper
  // perspective? (20040227 handegar)

  const SbVec2f slicecorners[4] = {
    SbVec2f(-2.0f,  2.0f),
    SbVec2f( 2.0f,  2.0f),
    SbVec2f( 2.0f, -2.0f),
    SbVec2f(-2.0f, -2.0f)
  };

  unsigned int nrofclippinginvocations = 0; // debug

  for (unsigned int i = 0; i < numslices; ++i) {

    if (this->abortfunc != NULL) { // Check user-callback status.
      SoVolumeRender::AbortCode abortcode =
        this->abortfunc(numslices, (numslices - i), this->abortfuncdata);
      if (abortcode == SoVolumeRender::ABORT) break;
      else if (abortcode == SoVolumeRender::SKIP) continue;
    }

    // FIXME: this doesn't stretch the slices all the way. 20040728 mortene.
    const float cam2slicedistance = neardistance + i*distancedelta;

    SbVec3f xformslicecorners[4];
    for (unsigned int j=0; j < 4; j++) {
      xformslicecorners[j] =
        viewvolume.getPlanePoint(cam2slicedistance, slicecorners[j]);
      mat.multVecMatrix(xformslicecorners[j], xformslicecorners[j]);
    }

    for (unsigned int cubeidx = 0; cubeidx < (unsigned int)subcubelist.getLength(); cubeidx++) {
      Cvr3DTexSubCubeItem * cubeitem = subcubelist[cubeidx];

      if (cam2slicedistance > (cubeitem->cameraplane2cubecenter + cubeitem->boundingsphereradius)) {
        continue; // We have passed the cube.
      }
      else if (cam2slicedistance < (cubeitem->cameraplane2cubecenter - cubeitem->boundingsphereradius)) {
        continue; // We haven't reached the cube yet.
      }

      cubeitem->cube->intersectSlice(xformslicecorners);
      nrofclippinginvocations++; // debug
    }
  }

#if 0 // debug
  if (CvrUtil::doDebugging()) {
    SoDebugError::postInfo("Cvr3DTexCube::render",
                           "Nr of SbClip invocations == %u",
                           nrofclippinginvocations);
  }
#endif // debug

  // Sort rendering order of the subcubes depending on the distance to
  // the camera.
  qsort((void *) subcubelist.getArrayPtr(), subcubelist.getLength(),
        sizeof(Cvr3DTexSubCubeItem *), subcube_qsort_compare);

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
  // Dummy vector based on the normal. Must not be equal to the normal
  // (or in the opposite direction). This will cause the normalization
  // of 'x' and 'z.cross(x)' to fail.
  SbVec3f y(z[1], -z[2], z[0]);
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
  viewvolume.ortho(-viewvolumesize, viewvolumesize, -viewvolumesize, viewvolumesize, 0.01f, 100.0f);
  viewvolume.transform(m);
  const SbMatrix mat = SoModelMatrixElement::get(state).inverse();

  SbList <Cvr3DTexSubCubeItem *> subcubelist;

  for (unsigned int rowidx = 0; rowidx < this->nrrows; rowidx++) {
    for (unsigned int colidx = 0; colidx < this->nrcolumns; colidx++) {
      for (unsigned int depthidx = 0; depthidx < this->nrdepths; depthidx++) {

        Cvr3DTexSubCube * cube = NULL;
        Cvr3DTexSubCubeItem * cubeitem = this->getSubCube(state, colidx, rowidx, depthidx);

        const SbVec3f subcubeorigo =
          this->origo +
          subcubewidth * (float)colidx +
          subcubeheight * (float)rowidx +
          subcubedepth * (float)depthidx;

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

  for (unsigned int rowidx = 0; rowidx < this->nrrows; rowidx++) {
    for (unsigned int colidx = 0; colidx < this->nrcolumns; colidx++) {
      for (unsigned int depthidx = 0; depthidx < this->nrdepths; depthidx++) {

        Cvr3DTexSubCube * cube = NULL;
        Cvr3DTexSubCubeItem * cubeitem = this->getSubCube(state, colidx, rowidx, depthidx);

        const SbVec3f subcubeorigo =
          this->origo +
          subcubewidth * (float)colidx +
          subcubeheight * (float)rowidx +
          subcubedepth * (float)depthidx;
        
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

  for (unsigned int rowidx = 0; rowidx < this->nrrows; rowidx++) {
    for (unsigned int colidx = 0; colidx < this->nrcolumns; colidx++) {
      for (unsigned int depthidx = 0; depthidx < this->nrdepths; depthidx++) {

        Cvr3DTexSubCube * cube = NULL;
        Cvr3DTexSubCubeItem * cubeitem = this->getSubCube(state, colidx, rowidx, depthidx);

        const SbVec3f subcubeorigo =
          this->origo +
          subcubewidth * (float)colidx +
          subcubeheight * (float)rowidx +
          subcubedepth * (float)depthidx;

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


unsigned int
Cvr3DTexCube::calcSubCubeIdx(unsigned int row, unsigned int col, unsigned int depth) const
{
  assert(row < this->nrrows);
  assert(col < this->nrcolumns);
  assert(depth < this->nrdepths);

  unsigned int idx =
    (col + (row * this->nrcolumns)) + (depth * this->nrcolumns * this->nrrows);

  assert(idx < (this->nrdepths * this->nrcolumns * this->nrrows));

  return idx;
}


// Builds a cube if it doesn't exist. Rebuilds it if it does exist.
Cvr3DTexSubCubeItem *
Cvr3DTexCube::buildSubCube(const SoGLRenderAction * action,
                           const SbVec3f & subcubeorigo,
                           unsigned int col, unsigned int row, unsigned int depth)
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
    for (unsigned int i=0; i < this->nrrows; i++) {
      for (unsigned int j=0; j < this->nrcolumns; j++) {
        for (unsigned int k=0; k < this->nrdepths; k++) {
          const unsigned int idx = this->calcSubCubeIdx(i, j, k);
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
  const SbBox3s subcubecut(subcubemin, subcubemax);
  const CvrTextureObject * texobj = CvrTextureObject::create(action, this->clut, subcubecut);
  // if NULL is returned, it means all voxels are fully transparent

  Cvr3DTexSubCube * cube = NULL;
  if (texobj) {
    cube = new Cvr3DTexSubCube(action, texobj, subcubeorigo,
                               subcubecut.getMax() - subcubecut.getMin());
    cube->setPalette(this->clut);
  }

  SoState * state = action->getState();
  const CvrVoxelBlockElement * vbelem = CvrVoxelBlockElement::getInstance(state);
  assert(vbelem != NULL);
  
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
  for (unsigned int col=0; col < this->nrcolumns; col++) {
    for (unsigned int row=0; row < this->nrrows; row++) {
      for (unsigned int depth = 0; depth < this->nrdepths; depth++) {
        const unsigned int idx = this->calcSubCubeIdx(row, col, depth);
        Cvr3DTexSubCubeItem * subc = this->subcubes[idx];

        // No cube was yet made in this position.
        if (subc == NULL) { continue; }

        // Only if invisible should there be no page allocated.
        assert(subc->invisible || subc->cube);

        // If this hits, the cube was RGBA and/or previously
        // invisible.  That may change when setting a new palette, so
        // remove the old cube.
        if (subc->invisible || !subc->cube->isPaletted()) {
          this->releaseSubCube(row, col, depth);
          continue;
        }

        // If paletted and previously visible, we simply migrate the new
        // palette to all sub-pages.
        subc->cube->setPalette(this->clut);
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
Cvr3DTexCube::getSubCube(SoState * state, unsigned int col, unsigned int row, unsigned int depth)
{
  if (this->subcubes == NULL) return NULL;

  assert(col < this->nrcolumns);
  assert(row < this->nrrows);
  assert(depth < this->nrdepths);

  const unsigned int idx = this->calcSubCubeIdx(row, col, depth);
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
