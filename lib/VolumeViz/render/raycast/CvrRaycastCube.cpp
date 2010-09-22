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
  the furure relies on raycasting for volemendering of gigabyte-sized
  datasets. This is why we don't just build on the old structure, but
  start fresh instead.

 */

#include "CvrRaycastCube.h"

#include <Inventor/SbLinear.h>
#include <Inventor/SoPath.h>
#include <Inventor/C/glue/gl.h>
#include <Inventor/system/gl.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoViewportRegionElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/elements/SoGLLazyElement.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/SbVec3s.h>
#include <Inventor/SbBox3s.h>
#include <Inventor/nodes/SoCube.h>

#include <VolumeViz/elements/CvrDataSizeElement.h>
#include <VolumeViz/elements/CvrVoxelBlockElement.h>
#include <VolumeViz/nodes/So2DTransferFunction.h>
#include <VolumeViz/misc/CvrUtil.h>

#include <VolumeViz/render/raycast/CvrRaycastSubCube.h>
#include <VolumeViz/render/common/CvrTextureObject.h>

#include <RenderManager.h>


class SubCube {
public:
  SubCube(CvrRaycastSubCube * p) { this->cube = p; }
  CvrRaycastSubCube * cube;
  double distancefromcamera;
  double cameraplane2cubecenter;
  float boundingsphereradius;    
  uint32_t volumedataid;
};


CvrRaycastCube::CvrRaycastCube(const SoGLRenderAction * action)
  : rendermanager(NULL), subcubes(NULL), 
    transferfunctionchanged(false), previoustransferfunctionid(0)
{
  SoState * state = action->getState();
  this->subcubesize = CvrUtil::clampSubCubeSize(CvrDataSizeElement::get(state));
  
  if (CvrUtil::doDebugging()) {
    SoDebugError::postInfo("CvrRaycastCube::CvrRaycastCube",
                           "subcubedimensions==<%d, %d, %d>",
                           this->subcubesize[0],
                           this->subcubesize[1],
                           this->subcubesize[2]);
  }

  this->previousviewportregion = SbViewportRegion(0, 0);
  
  const CvrVoxelBlockElement * vbelem = CvrVoxelBlockElement::getInstance(state);
  const SbVec3s & dim = vbelem->getVoxelCubeDimensions();
  assert(dim[0] > 0 && dim[1] > 0 && dim[2] > 0 && "Invalid voxel block size");

  this->dimensions = dim;
  this->origo = SbVec3f(-((float) dim[0]) / 2.0f, 
                        -((float) dim[1]) / 2.0f, 
                        -((float) dim[2]) / 2.0f);

  //Note: cols and rows were swiched in Cvr3DTexCube
  this->nrsubcubes[0] = (this->dimensions[0] + this->subcubesize[0] - 1) / this->subcubesize[0];
  this->nrsubcubes[1] = (this->dimensions[1] + this->subcubesize[1] - 1) / this->subcubesize[1];
  this->nrsubcubes[2] = (this->dimensions[2] + this->subcubesize[2] - 1) / this->subcubesize[2];
}


CvrRaycastCube::~CvrRaycastCube()
{
  this->releaseAllSubCubes();
}


void
CvrRaycastCube::setupRenderManager(const SoGLRenderAction * action)
{
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
 
  this->rendermanager = new CLVol::RenderManager(true);
}


void
CvrRaycastCube::adjustLayers(const SoGLRenderAction * action)
{
  SoState * state = action->getState();
  const SbViewportRegion & vpr = SoViewportRegionElement::get(state);
  SbVec2s size = vpr.getWindowSize();   
  
  const uint32_t glctxid = action->getCacheContext();
  const cc_glglue * glw = cc_glglue_instance(glctxid);

  // -- Target fbo
  glBindTexture(GL_TEXTURE_2D, this->gltargetcolorlayer);
  glTexImage2D(GL_TEXTURE_2D, 0,
               GL_RGBA,
               size[0], size[1], 0,
               GL_RGBA, GL_UNSIGNED_BYTE, NULL);  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  
  glBindTexture(GL_TEXTURE_2D, this->gltargetdepthlayer);
  glTexImage2D(GL_TEXTURE_2D, 0,
               GL_DEPTH_COMPONENT,
               //GL_DEPTH_COMPONENT32F,
               size[0], size[1], 0,
               GL_DEPTH_COMPONENT, 
               GL_FLOAT, NULL);
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
  glTexImage2D(GL_TEXTURE_2D, 0,
               GL_RGBA,
               size[0], size[1], 0,
               GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  
  glBindTexture(GL_TEXTURE_2D, this->gldepthlayers[0]);
  glTexImage2D(GL_TEXTURE_2D, 0,
               GL_DEPTH_COMPONENT,
               //GL_DEPTH_COMPONENT32F,
               size[0], size[1], 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, NULL );
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
  glTexImage2D(GL_TEXTURE_2D, 0,
               GL_RGBA,
               size[0], size[1], 0,
               GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  
  glBindTexture(GL_TEXTURE_2D, this->gldepthlayers[1]);
  glTexImage2D(GL_TEXTURE_2D, 0,
               GL_DEPTH_COMPONENT,
               //GL_DEPTH_COMPONENT32F,
                size[0], size[1], 0,
                GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
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


void 
CvrRaycastCube::render(const SoGLRenderAction * action)
{
  SoState * state = action->getState();

  // This must be done, as we want to control stuff in the GL state
  // machine. Without it, state changes could trigger outside our
  // control.
  SoGLLazyElement::getInstance(state)->send(state, SoLazyElement::ALL_MASK);

  glPushAttrib(GL_ALL_ATTRIB_BITS);
     
  const SbViewportRegion & vpr = SoViewportRegionElement::get(state);
  const bool reattachglresources = vpr != this->previousviewportregion;
  this->previousviewportregion = vpr;

  if (!this->rendermanager) {
    this->setupRenderManager(action);  
  }

  const SbVec2s size = vpr.getWindowSize();                 
  const SbVec3f subcubewidth(this->subcubesize[0], 0, 0);
  const SbVec3f subcubeheight(0, this->subcubesize[1], 0);
  const SbVec3f subcubedepth(0, 0, this->subcubesize[2]);
  const unsigned int startrow = 0, endrow = this->nrsubcubes[0] - 1;
  const unsigned int startcolumn = 0, endcolumn = this->nrsubcubes[1] - 1;
  const unsigned int startdepth = 0, enddepth = this->nrsubcubes[2] - 1;

  if (reattachglresources) {
    for (unsigned int rowidx = startrow; rowidx <= endrow; rowidx++) {
      for (unsigned int colidx = startcolumn; colidx <= endcolumn; colidx++) {
        for (unsigned int depthidx = startdepth; depthidx <= enddepth; depthidx++) {          
          SubCube * cubeitem = this->getSubCube(state, rowidx, colidx, depthidx);
          if (cubeitem) {            
            cubeitem->cube->detachGLResources();
            cubeitem->cube->setRenderTarget(0, // Target fbo=0 => Direct to screen
                                            0, 0, size[0], size[1]);            
          }
        }
      }
    } 
    this->adjustLayers(action);
  }

  const uint32_t glctxid = action->getCacheContext();
  const cc_glglue * glw = cc_glglue_instance(glctxid);
  const SbViewVolume adjustedviewvolume =  this->calculateAdjustedViewVolume(action);


  glEnable(GL_DEPTH_TEST);      
  // FIXME: Use glglue for EXT calls (20100914 handegar)
  cc_glglue_glBindFramebuffer(glw, GL_FRAMEBUFFER, this->gllayerfbos[1]);        
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
     

  cc_glglue_glBindFramebuffer(glw, GL_READ_FRAMEBUFFER, 0);
  cc_glglue_glBindFramebuffer(glw, GL_DRAW_FRAMEBUFFER, this->gllayerfbos[1]);
  // FIXME: glBlitFramebuffer is not bound by glue. Must fix in Coin. (20100914 handegar)
  glBlitFramebuffer(0, 0, size[0], size[1],
                    0, 0, size[0], size[1],
                    GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
  assert(glGetError() == GL_NO_ERROR);

  
  cc_glglue_glBindFramebuffer(glw, GL_READ_FRAMEBUFFER, 0);
  cc_glglue_glBindFramebuffer(glw, GL_DRAW_FRAMEBUFFER, this->gllayerfbos[0]);
  // FIXME: glBlitFramebuffer is not bound by glue. Must fix in Coin. (20100914 handegar)
  glBlitFramebuffer(0, 0, size[0], size[1],
                    0, 0, size[0], size[1],
                    GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
  assert(glGetError() == GL_NO_ERROR);
  


 
  for (unsigned int rowidx = startrow; rowidx <= endrow; rowidx++) {
    for (unsigned int colidx = startcolumn; colidx <= endcolumn; colidx++) {
      for (unsigned int depthidx = startdepth; depthidx <= enddepth; depthidx++) {
        
        SubCube * cubeitem = this->getSubCube(state, rowidx, colidx, depthidx);

        const SbVec3f subcubeorigo =
          this->origo +
          subcubewidth * (float)colidx + 
          subcubeheight * (float)rowidx + 
          subcubedepth * (float)depthidx;

        if (cubeitem == NULL) { 
          cubeitem = this->buildSubCube(action, subcubeorigo, rowidx, colidx, depthidx); 
          cubeitem->cube->setRenderTarget(0, // Target fbo=0 => Direct to screen
                                          0, 0, size[0], size[1]);
        }
        assert(cubeitem != NULL);
        assert(cubeitem->cube != NULL);

        if (reattachglresources) {         
          cubeitem->cube->attachGLLayers(this->glcolorlayers,
                                         this->gldepthlayers,
                                         size[0], size[1]);             
        }
        
        if (this->transferfunctionchanged) {
          cubeitem->cube->setTransferFunction(this->transferfunction);
        }


        // -- Do regular GL code here
        //  ... but how?
        // --------------------------
        
        // -- copy depth from solid pass into depth of transparent pass
        cc_glglue_glBindFramebuffer(glw, GL_READ_FRAMEBUFFER, this->gllayerfbos[1]);
        cc_glglue_glBindFramebuffer(glw, GL_DRAW_FRAMEBUFFER, this->gllayerfbos[0]);
        // FIXME: glBlitFramebuffer is not bound by glue. Must fix in Coin. (20100914 handegar)
        glBlitFramebuffer(0, 0, size[0], size[1],
                          0, 0, size[0], size[1],
                          GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        
        // -- transparency pass
        cc_glglue_glBindFramebuffer(glw, GL_FRAMEBUFFER, this->gllayerfbos[0]);
        glClear(GL_COLOR_BUFFER_BIT);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
        glDisable(GL_BLEND);

        cubeitem->cube->render(action, adjustedviewvolume); 
      }
    }
  }
  
 


  glPopAttrib();

  
  this->transferfunctionchanged = false;
}


const SbViewVolume
CvrRaycastCube::calculateAdjustedViewVolume(const SoGLRenderAction * action) const
{
  SoState * state = action->getState();
  
  SbBox3f bbox(this->origo, this->origo +
               SbVec3f(this->dimensions[0],
                       this->dimensions[1],
                       this->dimensions[2]));
  SbMatrix mm = SoModelMatrixElement::get(state);
  mm.setTranslate(SbVec3f(0.5, 0.5, 0.5));  
  mm.setScale(SbVec3f(1.0f / this->dimensions[0],
                      1.0f / this->dimensions[1],
                      1.0f / this->dimensions[2]));  
  bbox.transform(mm);

  // FIXME: Calculate the optimal sphere in which the bbox will fit
  // inside. Taking the diagonal will get a feasible result, but not
  // optimal.(20100920 handegar)
  const SbViewVolume & viewvolume = SoViewVolumeElement::get(state);
  const float bboxradius = bbox.getSize().length();
  const SbPlane camplane = viewvolume.getPlane(0.0f);
  const SbVec3f bboxcenter = bbox.getCenter();
  const float neardistance = SbAbs(camplane.getDistance(bboxcenter)) - bboxradius;
  const float fardistance = SbAbs(camplane.getDistance(bboxcenter)) + bboxradius;

  // We'll have to narrow the viewport near/far plane so that the
  // OpenCL-rendering won't get affected by other Coin objects in the
  // scenegraph.
  const SbViewVolume vv = SoViewVolumeElement::get(state);
  const double near = vv.getNearDist();
  const double depth = vv.getDepth();
  const double znarrownear = (neardistance - near) / depth;
  const double znarrowfar = ((fardistance-neardistance) - near) / depth;

  return vv.zNarrow(1.0f - znarrownear, 1.0f - znarrowfar);
}


SubCube * 
CvrRaycastCube::getSubCube(SoState * state, 
                           unsigned int row, 
                           unsigned int col, 
                           unsigned int depth)
{
  assert(row < this->nrsubcubes[0]);
  assert(col < this->nrsubcubes[1]);
  assert(depth < this->nrsubcubes[2]);
  if (this->subcubes == NULL) 
    return NULL;

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


SubCube * 
CvrRaycastCube::buildSubCube(const SoGLRenderAction * action,
                             const SbVec3f & origo,
                             unsigned int row,
                             unsigned int col,
                             unsigned int depth)
{
  assert((this->getSubCube(action->getState(), row, col, depth) == NULL) && 
         "Subcube already created!");

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
        
  SbVec3s subcubemin, subcubemax;
  subcubemin = SbVec3s(col * this->subcubesize[0],
                       row * this->subcubesize[1],
                       depth * this->subcubesize[2]);
  subcubemax = SbVec3s((col + 1) * this->subcubesize[0],
                       (row + 1) * this->subcubesize[1],
                       (depth + 1) * this->subcubesize[2]);
  
  // Crop subcube size
  subcubemax[0] = SbMin(subcubemax[0], this->dimensions[0]);
  subcubemax[1] = SbMin(subcubemax[1], this->dimensions[1]);
  subcubemax[2] = SbMin(subcubemax[2], this->dimensions[2]);

  subcubemin[1] = SbMax(subcubemin[1], (short) 0);

#if CVR_DEBUG && 0 // debug
  SoDebugError::postInfo("CvrRaycastCube::buildSubCube",
                         "subcubemin=[%d, %d, %d] subcubemax=[%d, %d, %d]",
                         subcubemin[0], subcubemin[1], subcubemin[2],
                         subcubemax[0], subcubemax[1], subcubemax[2]);
#endif // debug

  const SbBox3s subcubecut(subcubemin, subcubemax);
  const CvrTextureObject * texobj = 
    CvrTextureObject::create(action, NULL, subcubecut);
  // if NULL is returned, it means all voxels are fully transparent
  
  CvrRaycastSubCube * cube = NULL;
  if (texobj) {
    cube = new CvrRaycastSubCube(action, texobj, origo,
                                 subcubecut.getMax() - subcubecut.getMin(),
                                 this->rendermanager);
  }

  SoState * state = action->getState();
  const CvrVoxelBlockElement * vbelem = CvrVoxelBlockElement::getInstance(state);
  assert(vbelem != NULL);
  
  SubCube * item = new SubCube(cube);
  item->volumedataid = vbelem->getNodeId();

  const int idx = this->getSubCubeIdx(row, col, depth);
  this->subcubes[idx] = item;

  return item;
}


unsigned int
CvrRaycastCube::getSubCubeIdx(unsigned int row, 
                              unsigned int col, 
                              unsigned int depth) const
{
  assert(row < this->nrsubcubes[0]);
  assert(col < this->nrsubcubes[1]);
  assert(depth < this->nrsubcubes[2]);

  unsigned int idx =
    (col + (row * this->nrsubcubes[1])) + (depth * this->nrsubcubes[0] * this->nrsubcubes[1]);
  
  assert(idx < (this->nrsubcubes[0] * this->nrsubcubes[1] * this->nrsubcubes[2]));
  return idx;
}


void 
CvrRaycastCube::releaseSubCube(unsigned int row, 
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
CvrRaycastCube::releaseAllSubCubes()
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


void 
CvrRaycastCube::setTransferFunction(So2DTransferFunction * tf)
{
  // Has this transfer function changes since last time?
  if (!tf || (tf->getNodeId() == this->previoustransferfunctionid))
    return;

  const int numcols = tf->colors.getNum();
  const int numpts = tf->colorPoints.getNum();
  const int min = numcols > numpts ? numpts : numcols;
  
  this->transferfunction.clear();  
  for (int i=0;i<min;++i) {
    CLVol::TransferFunctionPoint tfp;
    tfp.value = tf->colorPoints[i];
    SbColor4f c = tf->colors[i];
    tfp.rgba[0] = c[0];
    tfp.rgba[1] = c[1];
    tfp.rgba[2] = c[2];
    tfp.rgba[3] = c[3];
    this->transferfunction.push_back(tfp);
  }
  
  this->transferfunctionchanged = true;
  this->previoustransferfunctionid = tf->getNodeId();
}
