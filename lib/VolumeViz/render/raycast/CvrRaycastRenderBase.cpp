/**************************************************************************\
 *
 *  This file is part of the SIM Voleon visualization library.
 *  Copyright (C) by Kongsberg Oil & Gas Technologies.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  ("GPL") version 2 as published by the Free Software Foundation.
 *  See the file LICENSE.GPL at the root directory of this source
 *  distribution for additional information about the GNU GPL.
 *
 *  For using SIM Voleon with software that can not be combined with
 *  the GNU GPL, and for taking advantage of the additional benefits
 *  of our support services, please contact Kongsberg Oil & Gas
 *  Technologies about acquiring a SIM Voleon Professional Edition
 *  License.
 *
 *  See http://www.coin3d.org/ for more information.
 *
 *  Kongsberg Oil & Gas Technologies, Bygdoy Alle 5, 0257 Oslo, NORWAY.
 *  http://www.sim.no/  sales@sim.no  coin-support@coin3d.org
 *
\**************************************************************************/

/*!
  This class is used for setting up and handling subcube-textures
  which will be used for rendering by the libCLVol library.

  NOTE: A lot of this code is copied from and inspired by the
  Cvr3DTexCube class, but somewhat refined. The code in Cvr3DTexCube
  will, in the long run, become obsolete when SIMVoleon somewhere in
  the future relies on raycasting for volemendering of gigabyte-sized
  datasets. This is why we don't just build on the old structure, but
  start fresh instead.
 */

#include "CvrRaycastRenderBase.h"

#include <Inventor/SbLinear.h>
#include <Inventor/C/glue/gl.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoViewportRegionElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/SbVec3s.h>
#include <Inventor/SbBox3s.h>

#include <VolumeViz/elements/CvrDataSizeElement.h>
#include <VolumeViz/elements/CvrVoxelBlockElement.h>
#include <VolumeViz/nodes/So2DTransferFunction.h>
#include <VolumeViz/misc/CvrUtil.h>
#include <VolumeViz/misc/CvrBBoxSubdivider.h>
#include <VolumeViz/render/raycast/CvrRaycastSubCube.h>
#include <VolumeViz/render/raycast/CvrRaycastTexture.h>

#include <RenderManager.h>
#include <TransferFunction.h>

// FIXME: The rendermanager is never released. (20101108 handegar)
CLVol::RenderManager * CvrRaycastRenderBase::rendermanager = NULL;


static int
subcube_qsort_compare(const void * element1, const void * element2)
{
  SubCube ** sc1 = (SubCube **) element1;
  SubCube ** sc2 = (SubCube **) element2;
  return ((*sc1)->distancefromcamera > (*sc2)->distancefromcamera) ? -1 : 1;
}


CvrRaycastRenderBase::CvrRaycastRenderBase(const SoGLRenderAction * action)
  : subcubes(NULL), 
    transferfunctionchanged(false), previoustransferfunctionid(0),
    cltransferfunction(NULL)
{
  SoState * state = action->getState();
  this->previousviewportregion = SbViewportRegion(0, 0);

  const SbVec3s subcubesize = CvrUtil::clampSubCubeSize(CvrDataSizeElement::get(state));
  
  if (CvrUtil::doDebugging()) {
    SoDebugError::postInfo("CvrRaycastRenderBase::CvrRaycastRenderBase",
                           "subcubedimensions==<%d, %d, %d>",
                           subcubesize[0],
                           subcubesize[1],
                           subcubesize[2]);
  }
  
  const CvrVoxelBlockElement * vbelem = CvrVoxelBlockElement::getInstance(state);
  const SbVec3s & dim = vbelem->getVoxelCubeDimensions();
  assert(dim[0] > 0 && dim[1] > 0 && dim[2] > 0 && "Invalid voxel block size");

  this->dimensions = dim;
  this->origo = -SbVec3f(dim[0] / 2.0f, dim[1] / 2.0f, dim[2] / 2.0f);
  
  const SbBox3f cubebox(SbVec3f(0, 0, 0), SbVec3f(dim[0], dim[1], dim[2]));
  const SbBox3f subcubebox(SbVec3f(0, 0, 0), 
                           SbVec3f(subcubesize[0], subcubesize[1], subcubesize[2]));
    
  // FIXME: What if the dataset changes? Won't this have to be
  // recalculated? (20100928 handegar)
  CvrBBoxSubdivider bbs;
  SbBox3f b = subcubebox;
  SbMatrix t = SbMatrix::identity();
  t.setTranslate(SbVec3f(subcubesize[0], 0, 0));
  this->nrsubcubes[0] = bbs.countIntersectingBoxesInDirection(cubebox, b, t);
  b = subcubebox;
  t.setTranslate(SbVec3f(0, subcubesize[1], 0));
  this->nrsubcubes[1] = bbs.countIntersectingBoxesInDirection(cubebox, b, t);
  b = subcubebox;
  t.setTranslate(SbVec3f(0, 0, subcubesize[2]));
  this->nrsubcubes[2] = bbs.countIntersectingBoxesInDirection(cubebox, b, t);
  bbs.collectAllBoxesInside(cubebox, subcubebox, this->subcubeboxes);


  this->glcolorlayers.resize(2);
  this->gldepthlayers.resize(2);
  this->gllayerfbos.resize(2);
  
  const uint32_t glctxid = action->getCacheContext();
  const cc_glglue * glw = cc_glglue_instance(glctxid);
  
  glGenTextures(2, &this->glcolorlayers[0]);
  glGenTextures(2, &this->gldepthlayers[0]);
  cc_glglue_glGenFramebuffers(glw, 2, &this->gllayerfbos[0]);
  
  cc_glglue_glGenFramebuffers(glw, 1, &this->gltargetfbo);
  glGenTextures(1, &this->gltargetcolorlayer);
  glGenTextures(1, &this->gltargetdepthlayer);
  assert(glGetError() == GL_NO_ERROR);
}


CvrRaycastRenderBase::~CvrRaycastRenderBase()
{
  this->releaseAllSubCubes();
  
  // FIXME: Can we guarantee that a value OpenGL context is active
  // here? (20101111 handegar)
  glDeleteTextures(2, &this->glcolorlayers[0]);
  glDeleteTextures(2, &this->gldepthlayers[0]); 

  // FIXME: Release the framebuffers created in the constructor here
  // (we'll need an SoAction to be able to use the cc_clglue_*
  // methods.) (20101111 handegar)

  if (CvrRaycastRenderBase::rendermanager) {
    CvrRaycastRenderBase::rendermanager->releaseTransferFunction(this->cltransferfunction);
    delete CvrRaycastRenderBase::rendermanager;
  }
}


void 
CvrRaycastRenderBase::sortSubCubes(SbList<SubCube *> & subcubelist) const
{
  qsort((void *) subcubelist.getArrayPtr(), subcubelist.getLength(),
        sizeof(SubCube *), subcube_qsort_compare);
}


void 
CvrRaycastRenderBase::setTransferFunction(So2DTransferFunction * tf)
{
  // Has this transfer function changes since last time?
  if (!tf || (tf->getNodeId() == this->previoustransferfunctionid)) {
    return;
  }

  const int numcols = tf->colors.getNum();
  const int numpts = tf->colorPoints.getNum();
  const int min = numcols > numpts ? numpts : numcols;
  
  this->transferfunctionpoints.clear();
  for (int i=0;i<min;++i) {
    this->transferfunctionpoints.push_back(tf->colorPoints[i]);
    SbColor4f c = tf->colors[i];
    for (int j=0;j<4;++j)
      this->transferfunctionpoints.push_back(c[j]);
  }
  
  this->transferfunctionchanged = true;
  this->previoustransferfunctionid = tf->getNodeId();
}


SubCube * 
CvrRaycastRenderBase::buildSubCube(const SoGLRenderAction * action,
                                   unsigned int row,
                                   unsigned int col,
                                   unsigned int depth)
{
  assert((this->getSubCube(action->getState(), row, col, depth) == NULL) && 
         "Subcube already created!");
  
  assert(CvrRaycastRenderBase::rendermanager && "RenderManager not initialized");

  // No subcubes created yet?
  if (this->subcubes == NULL) {
    if (CvrUtil::doDebugging()) {
      SoDebugError::postInfo("CvrRaycastCube::buildSubCube",
                             "number of subcubes needed == %d (%d x %d x %d)",
                             this->nrsubcubes[0]*this->nrsubcubes[1]*this->nrsubcubes[2],
                             this->nrsubcubes[0], this->nrsubcubes[1], this->nrsubcubes[2]);
    }

    this->subcubes = new SubCube*[this->nrsubcubes[0]*this->nrsubcubes[1]*this->nrsubcubes[2]];
    for (unsigned int i=0; i < this->nrsubcubes[0]; i++) {
      for (unsigned int j=0; j < this->nrsubcubes[1]; j++) {
        for (unsigned int k=0; k < this->nrsubcubes[2]; k++) {
          const unsigned int idx = this->getSubCubeIdx(i, j, k);
          this->subcubes[idx] = NULL; // Reset to NULL
        }
      }
    }
  }
   

  const int idx = this->getSubCubeIdx(row, col, depth);
  assert(idx < this->subcubeboxes.getLength());

  const SbBox3s subcubecut(this->subcubeboxes[idx]);
  const CvrRaycastTexture * texobj = 
    CvrRaycastTexture::create(CvrRaycastRenderBase::rendermanager, action, subcubecut);
  texobj->ref(); // FIXME: Never unref'ed! (20101008 handegar)

  CvrRaycastSubCube * cube = new CvrRaycastSubCube(action, texobj, 
                                                   subcubecut,
                                                   this->dimensions,
                                                   CvrRaycastRenderBase::rendermanager);

  SoState * state = action->getState();
  const CvrVoxelBlockElement * vbelem = CvrVoxelBlockElement::getInstance(state);
  assert(vbelem != NULL);
  
  SubCube * item = new SubCube(cube);
  item->bbox = this->subcubeboxes[idx];
  item->volumedataid = vbelem->getNodeId();

  this->subcubes[idx] = item;
  return item;
}


unsigned int
CvrRaycastRenderBase::getSubCubeIdx(unsigned int row, 
                                    unsigned int col, 
                                    unsigned int depth) const
{
  assert(row < this->nrsubcubes[0]);
  assert(col < this->nrsubcubes[1]);
  assert(depth < this->nrsubcubes[2]);

  unsigned int idx = (col + (row * this->nrsubcubes[1])) + 
    (depth * this->nrsubcubes[0] * this->nrsubcubes[1]);
  
  assert(idx < (this->nrsubcubes[0] * this->nrsubcubes[1] * this->nrsubcubes[2]));
  return idx;
}


SubCube * 
CvrRaycastRenderBase::getSubCube(SoState * state, 
                                 unsigned int row, 
                                 unsigned int col, 
                                 unsigned int depth)
{
  assert(row < this->nrsubcubes[0]);
  assert(col < this->nrsubcubes[1]);
  assert(depth < this->nrsubcubes[2]);
  if (this->subcubes == NULL) return NULL;

  const unsigned int idx = this->getSubCubeIdx(row, col, depth);
  SubCube * subp = this->subcubes[idx];

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
  else {
    // No subcube created yet for this row, col, depth.
  }

  return subp;
}


void 
CvrRaycastRenderBase::releaseSubCube(unsigned int row, 
                                     unsigned int column, 
                                     unsigned int depth)
{
  const int idx = this->getSubCubeIdx(row, column, depth);
  SubCube * p = this->subcubes[idx];
  if (p) {
    this->subcubes[idx] = NULL;
    delete p->cube;
    delete p;
  }
}


void
CvrRaycastRenderBase::releaseAllSubCubes()
{
  if (this->subcubes == NULL) return;
  
  for (unsigned int row = 0; row < this->nrsubcubes[0]; row++) {
    for (unsigned int col = 0; col < this->nrsubcubes[1]; col++) {
      for (unsigned int depth = 0; depth < this->nrsubcubes[2]; depth++) {
        this->releaseSubCube(row, col, depth);
      }
    }
  }
  
  delete[] this->subcubes;
  this->subcubes = NULL;
}


SbVec3ui32 
CvrRaycastRenderBase::getNumberOfSubcubes() const
{
  return this->nrsubcubes;
}


CLVol::RenderManager * 
CvrRaycastRenderBase::getRenderManager(const SoGLRenderAction * action)
{  
  this->setupRenderManager(action);  
  return CvrRaycastRenderBase::rendermanager;
}


void
CvrRaycastRenderBase::setupRenderManager(const SoGLRenderAction * action)
{
  // FIXME: This might be too primitive. A centralized mechanism for
  // handling the rendermanager should be made for more flexible
  // setups. (20101108 handegar)
  if (!CvrRaycastRenderBase::rendermanager) {
    CvrRaycastRenderBase::rendermanager = new CLVol::RenderManager(true);
  }
}



void
CvrRaycastRenderBase::adjustLayers(const SoGLRenderAction * action)
{
  SoState * state = action->getState();
  const SbViewportRegion & vpr = SoViewportRegionElement::get(state);
  SbVec2s size = vpr.getWindowSize();   
  
  const uint32_t glctxid = action->getCacheContext();
  const cc_glglue * glw = cc_glglue_instance(glctxid);

  // -- Target fbo
  glBindTexture(GL_TEXTURE_2D, this->gltargetcolorlayer);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size[0], size[1], 
               0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  
  glBindTexture(GL_TEXTURE_2D, this->gltargetdepthlayer);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, size[0], size[1], 
               0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  

  cc_glglue_glBindFramebuffer(glw, GL_FRAMEBUFFER, this->gltargetfbo);
  cc_glglue_glFramebufferTexture2D(glw, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D, this->gltargetcolorlayer, 0);
  cc_glglue_glFramebufferTexture2D(glw, GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                   GL_TEXTURE_2D, this->gltargetdepthlayer, 0);
  assert(glGetError() == GL_NO_ERROR);

  // -- solid layer
  glBindTexture(GL_TEXTURE_2D, this->glcolorlayers[0]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size[0], size[1], 
               0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  
  glBindTexture(GL_TEXTURE_2D, this->gldepthlayers[0]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, size[0], size[1], 
               0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  
  cc_glglue_glBindFramebuffer(glw, GL_FRAMEBUFFER, this->gllayerfbos[0]);
  cc_glglue_glFramebufferTexture2D(glw, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D, this->glcolorlayers[0], 0);
  cc_glglue_glFramebufferTexture2D(glw, GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                   GL_TEXTURE_2D, this->gldepthlayers[0], 0);
  
  // -- transparency layer
  glBindTexture(GL_TEXTURE_2D, this->glcolorlayers[1]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size[0], size[1], 
               0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  
  glBindTexture(GL_TEXTURE_2D, this->gldepthlayers[1]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, size[0], size[1], 
               0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  
  cc_glglue_glBindFramebuffer(glw, GL_FRAMEBUFFER, this->gllayerfbos[1]);
  cc_glglue_glFramebufferTexture2D(glw, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D, this->glcolorlayers[1], 0);
  cc_glglue_glFramebufferTexture2D(glw, GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                   GL_TEXTURE_2D, this->gldepthlayers[1], 0);

  cc_glglue_glBindFramebuffer(glw, GL_FRAMEBUFFER, 0);
}


SbList<SubCube *>
CvrRaycastRenderBase::processSubCubes(const SoGLRenderAction * action)
{
  SoState * state = action->getState();
  const SbViewportRegion & vpr = SoViewportRegionElement::get(state);
  const bool reattachglresources = vpr != this->previousviewportregion;
  this->previousviewportregion = vpr;

  const SbVec2s size = vpr.getWindowSize();  
  const SbVec3ui32 nrsubcubes = this->getNumberOfSubcubes();
  const unsigned int startrow = 0, endrow = nrsubcubes[0] - 1;
  const unsigned int startcolumn = 0, endcolumn = nrsubcubes[1] - 1;
  const unsigned int startdepth = 0, enddepth = nrsubcubes[2] - 1;

  CLVol::RenderManager * rendermanager = this->getRenderManager(action);

  rendermanager->detachGLResources();   
  rendermanager->setRenderTarget(0, // Target fbo=0 => Direct to screen
                                 0, 0, size[0], size[1]);
  
  if (reattachglresources) {
    this->adjustLayers(action);
  }

  rendermanager->attachGLLayers(this->glcolorlayers, this->gldepthlayers,
                                size[0], size[1]);


  if (!this->cltransferfunction) {
    // FIXME: Move this stuff to a place more suitable (20101225 handegar)
    this->cltransferfunction = CvrRaycastRenderBase::rendermanager->createTransferFunction(); 
  }

  // Update the transferfunction while we're at it.
  if (this->transferfunctionchanged) { 
    assert(this->cltransferfunction);
    this->cltransferfunction->setTransferFunction(this->transferfunctionpoints);
    this->transferfunctionchanged = false; 
  }

  rendermanager->bindTransferFunction(this->cltransferfunction);

  //
  // Sort the subcubes according to distance from the camera
  //
  SbList<SubCube *> subcuberenderorder;
  SbViewVolume viewvolume = SoViewVolumeElement::get(state);
  SbViewVolume viewvolumeinv = viewvolume;
  viewvolumeinv.transform(SoModelMatrixElement::get(state).inverse());
  SbMatrix bboxtrans;
  bboxtrans.setTranslate(this->origo);

  const SbPlane invcamplane = viewvolumeinv.getPlane(0.0f);

  for (unsigned int rowidx = startrow; rowidx <= endrow; rowidx++) {
    for (unsigned int colidx = startcolumn; colidx <= endcolumn; colidx++) {
      for (unsigned int depthidx = startdepth; depthidx <= enddepth; depthidx++) {
        SubCube * cubeitem = this->getSubCube(state, rowidx, colidx, depthidx);
        if (cubeitem == NULL)
          cubeitem = this->buildSubCube(action, rowidx, colidx, depthidx);           
        
        SbBox3f bbox = cubeitem->bbox;
        bbox.transform(bboxtrans);

        // FIXME: Do a check if the bbox is inside the frustrum at all
        // here? (20100930 handegar)
        
        const SbVec3f point = viewvolumeinv.getProjectionPoint();
        const float dist = (point - bbox.getClosestPoint(point)).length();
        cubeitem->distancefromcamera = dist;          
        if (invcamplane.getDistance(bbox.getCenter()) >= 0)
          cubeitem->distancefromcamera = -dist;         
        
        subcuberenderorder.append(cubeitem);       
      }
    }
  }

  this->sortSubCubes(subcuberenderorder);

  return subcuberenderorder;
}
