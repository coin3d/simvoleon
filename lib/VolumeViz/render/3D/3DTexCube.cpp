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

#include <limits.h>
#include <string.h>

#include <Inventor/C/tidbits.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/system/gl.h>
#include <Inventor/SbLinear.h>
#include <Inventor/SbViewVolume.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoProjectionMatrixElement.h>
#include <Inventor/elements/SoViewingMatrixElement.h>

#include <VolumeViz/render/3D/Cvr3DTexCube.h>

#include <VolumeViz/elements/SoTransferFunctionElement.h>
#include <VolumeViz/elements/SoVolumeDataElement.h>
#include <VolumeViz/misc/CvrUtil.h>
#include <VolumeViz/misc/CvrVoxelChunk.h>
#include <VolumeViz/misc/CvrCLUT.h>
#include <VolumeViz/nodes/SoTransferFunction.h>
#include <VolumeViz/nodes/SoVolumeData.h>
#include <VolumeViz/render/common/CvrRGBATexture.h>
#include <VolumeViz/render/common/CvrPaletteTexture.h>
#include <VolumeViz/render/common/Cvr3DRGBATexture.h>
#include <VolumeViz/render/common/Cvr3DPaletteTexture.h>


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

Cvr3DTexCube::Cvr3DTexCube(SoVolumeReader * reader)
{

  this->clut = NULL;
  this->subcubes = NULL;

  this->subcubesize = SbVec3s(0, 0, 0);
  calculateOptimalSubCubeSize();

  this->reader = reader;

  SbVec3s dim;
  SbBox3f size;
  SoVolumeData::DataType dummy;
  this->reader->getDataChar(size, dummy, dim);

  assert(dim[0] > 0);
  assert(dim[1] > 0);
  assert(dim[2] > 0);

  float width, height, depth;
  size.getSize(width, height, depth);
  this->dimensions = dim;

  this->nrcolumns = (this->dimensions[0] + this->subcubesize[0] - 1) / this->subcubesize[0];
  this->nrrows = (this->dimensions[1] + this->subcubesize[1] - 1) / this->subcubesize[1];
  this->nrdepths = (this->dimensions[2] + this->subcubesize[2] - 1) / this->subcubesize[2];

  this->rendersubcubeoutline = FALSE;
  const char * envstr = coin_getenv("CVR_SUBCUBE_FRAMES");
  if (envstr) { this->rendersubcubeoutline = atoi(envstr) > 0 ? TRUE : FALSE; }

}

Cvr3DTexCube::~Cvr3DTexCube()
{
  this->releaseAllSubCubes();
  if (this->clut) { this->clut->unref(); }
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

void
Cvr3DTexCube::calculateOptimalSubCubeSize()
{

  GLint maxsize;
  glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &maxsize);

  short forcedsubcubesize = 0;
  const char * envstr = coin_getenv("CVR_FORCE_SUBCUBE_SIZE");
  if (envstr) { forcedsubcubesize = atoi(envstr); }

  if (coin_is_power_of_two(forcedsubcubesize)) {
    if (forcedsubcubesize <= maxsize)
      maxsize = forcedsubcubesize;
  }
  else {
    if (forcedsubcubesize != 0)
      SoDebugError::postWarning("calculateOptimalSubCubeSize",
                                "Forced subcube size (%d) is not power of two.",
                                forcedsubcubesize);
  }

  // FIXME: My GeforceFX 5600 card sometime fails when asking for 512 as
  // cube size even if it is supposed to handle it. (20040302 handegar)
  //maxsize = SbMin(256, maxsize);

  this->subcubesize = SbVec3s(maxsize, maxsize, maxsize);

}

// Renders arbitrary positioned quad, textured for the cube (slice)
// represented by this object. Automatically loads all cubes needed.
void
Cvr3DTexCube::render(SoGLRenderAction * action,
                     const SbVec3f & origo,
                     const SbVec3f & cubespan,
                     Cvr3DTexSubCube::Interpolation interpolation,
                     unsigned int numslices)
{
  const cc_glglue * glglue = cc_glglue_instance(action->getCacheContext());

  SoState * state = action->getState();
  const SbMatrix & projmat = (SoModelMatrixElement::get(state) *
                              SoViewingMatrixElement::get(state) *
                              SoProjectionMatrixElement::get(state));

  SbVec3f subcubewidth = SbVec3f(cubespan[0] * this->subcubesize[0], 0, 0);
  SbVec3f subcubeheight = SbVec3f(0, cubespan[1] * this->subcubesize[1], 0);
  SbVec3f subcubedepth = SbVec3f(0, 0, cubespan[2] * this->subcubesize[2]);

  const SbViewVolume & viewvolume = SoViewVolumeElement::get(action->getState());
  const SbBox3f bbox(origo, origo +
                     SbVec3f(this->dimensions[0], this->dimensions[1], this->dimensions[2]));

  float dx,dy,dz;
  bbox.getSize(dx, dy, dz);
  const float bboxradius = sqrtf((dx/2) * (dy/2) * (dz/2));
  const SbPlane camplane = viewvolume.getPlane(0.0f);
  const SbVec3f bboxcenter = bbox.getCenter();
  const float neardistance = SbAbs(camplane.getDistance(bboxcenter)) + bboxradius;
  const float fardistance = SbAbs(camplane.getDistance(bboxcenter)) - bboxradius;
  const float distancedelta = (fardistance - neardistance) / numslices;

  SbList <Cvr3DTexSubCubeItem *> subcubelist;

  for (int rowidx = 0; rowidx < this->nrrows; rowidx++) {
    for (int colidx = 0; colidx < this->nrcolumns; colidx++) {
      for (int depthidx = 0; depthidx < this->nrdepths; depthidx++) {

        Cvr3DTexSubCube * cube = NULL;
        Cvr3DTexSubCubeItem * cubeitem = this->getSubCube(state, colidx, rowidx, depthidx);
        if (cubeitem == NULL) {
          cubeitem = this->buildSubCube(action, colidx, rowidx, depthidx, cubespan);
        }
        assert(cubeitem != NULL);
        if (cubeitem->invisible) continue;
        assert(cubeitem->cube != NULL);


        SbVec3f subcubeorigo = origo + // horizontal shift to correct column
          subcubewidth * colidx + // vertical shift to correct row
          subcubeheight * rowidx + // depth shift
          subcubedepth * depthidx;

        const SbVec3f subcubecenter = subcubeorigo +
          (subcubewidth/2 + subcubeheight/2 + subcubedepth/2);

        const float cubedist = (viewvolume.getProjectionPoint() - subcubecenter).length();
        const float cubeplanedist = SbAbs(camplane.getDistance(subcubecenter));
        const float cuberadius = (subcubewidth + subcubeheight + subcubedepth).length()/2;
        cubeitem->cube->setDistanceFromCamera(cubedist);

        for (unsigned int i=0;i<numslices;++i) {
          // Check if the cutplane intersect the bounding sphere of the cube at all.
          const float dist = fardistance - i*distancedelta;
          if (dist > (cubeplanedist+cuberadius)) break; // We have passed the cube.
          if (dist < (cubeplanedist-cuberadius)) continue; // We havent reached the cube yet.
          cubeitem->cube->checkIntersectionSlice(subcubeorigo, viewvolume, dist);
        }

        subcubelist.append(cubeitem);

      }
    }
  }

  qsort((void *) subcubelist.getArrayPtr(),
        subcubelist.getLength(),
        sizeof(Cvr3DTexSubCubeItem *),
        subcube_qsort_compare);

  for (int i=0;i<subcubelist.getLength();++i)
    subcubelist[i]->cube->render(action, interpolation);

  // Draw lines around each subcube.
  if (this->rendersubcubeoutline) {
    for (int i=0;i<subcubelist.getLength();++i)
      subcubelist[i]->cube->renderBBox(action, i);
  }

  subcubelist.truncate(0);

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
Cvr3DTexCube::buildSubCube(SoGLRenderAction * action, int col, int row, int depth, SbVec3f cubescale)
{
  // FIXME: optimalization idea; *crop* textures for 100%
  // transparency. 20021124 mortene.

  // FIXME: optimalization idea; detect 100% similar neighboring
  // cubes, and make cubes able to map to several "slice indices". Not
  // sure if this can be much of a gain -- but look into it. 20021124 mortene.

  assert(this->getSubCube(action->getState(), col, row, depth) == NULL);

  // First Cvr3DTexSubCube ever in this slice?
  if (this->subcubes == NULL) {
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

  // NOTE: Building subcubes 'upwards' so that the Y orientation will
  // be equal as the 2D slice rendering (the voxelchunks are also
  // flipped).

  SbVec3s subcubemin(col * this->subcubesize[0],
                     this->dimensions[1] - (row + 1) * this->subcubesize[1],
                     depth * this->subcubesize[2]);
  SbVec3s subcubemax((col + 1) * this->subcubesize[0],
                     this->dimensions[1] - row * this->subcubesize[1],
                     (depth + 1) * this->subcubesize[2]);

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
  const SoVolumeDataElement * vdelement = SoVolumeDataElement::getInstance(state);
  assert(vdelement != NULL);
  SoVolumeData * voldatanode = vdelement->getVolumeData();
  assert(voldatanode != NULL);

  SbVec3s vddims;
  void * dataptr;
  SoVolumeData::DataType type;
  SbBool ok = voldatanode->getVolumeData(vddims, dataptr, type);
  assert(ok);

  CvrVoxelChunk::UnitSize vctype;
  switch (type) {
  case SoVolumeData::UNSIGNED_BYTE: vctype = CvrVoxelChunk::UINT_8; break;
  case SoVolumeData::UNSIGNED_SHORT: vctype = CvrVoxelChunk::UINT_16; break;
  case SoVolumeData::RGBA: vctype = CvrVoxelChunk::UINT_32; break;
  default: assert(FALSE); break;
  }

  // FIXME: improve buildSubCube() interface to fix this roundabout
  // way of calling it. 20021206 mortene.
  CvrVoxelChunk * input = new CvrVoxelChunk(vddims, vctype, dataptr);
  CvrVoxelChunk * cubechunk = input->buildSubCube(subcubecut);
  delete input;

  // FIXME: optimalization measure; should be able to save on texture
  // memory by not using full cubes where only parts of them are
  // actually covered by texture (volume data does more often than not
  // fail to match dimensions perfectly with 2^n values). 20021125 mortene.

  SbBool invisible;
  CvrTextureObject * texobj = cubechunk->transfer3D(action, invisible);
  delete cubechunk;

#if CVR_DEBUG && 0 // debug
  SoDebugError::postInfo("Cvr3DTexCube::buildSubCube",
                         "detected invisible cube at [%d, %d, %d]", row, col, depth);
#endif // debug

  // Size of the texture that we're actually using. Will be less than
  // this->subcubesize on datasets where dimensions are not all power
  // of two, or where dimensions are smaller than this->subcubesize.
  const SbVec3s texsize(subcubemax - subcubemin);

  // Must clear the unused texture area to prevent artifacts due to
  // inaccuracies when calculating texture coords.
  if (texobj->getTypeId() == Cvr3DRGBATexture::getClassTypeId()) {
    ((Cvr3DRGBATexture *) texobj)->blankUnused(texsize);
  } else if (texobj->getTypeId() == Cvr3DPaletteTexture::getClassTypeId()) {
    ((Cvr3DPaletteTexture *) texobj)->blankUnused(texsize);
  }

  Cvr3DTexSubCube * cube = NULL;
  if (!invisible) {
    short dx, dy, dz;
    subcubecut.getSize(dx, dy, dz);
    const SbVec3f cubesize(cubescale[0] * dx, cubescale[1] * dy, cubescale[2] * dz);
    cube = new Cvr3DTexSubCube(action, texobj, cubesize, texsize,
                               voldatanode->useCompressedTexture.getValue());
    cube->setPalette(this->clut);
  }

  delete texobj;

  Cvr3DTexSubCubeItem * pitem = new Cvr3DTexSubCubeItem(cube);
  pitem->volumedataid = voldatanode->getNodeId();
  pitem->invisible = invisible;

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
    const SoVolumeData * volumedata = SoVolumeDataElement::getInstance(state)->getVolumeData();
    uint32_t volumedataid = volumedata->getNodeId();

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
