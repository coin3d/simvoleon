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

#include <Inventor/C/tidbits.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoGLLazyElement.h>
#include <Inventor/bundles/SoMaterialBundle.h>
#include <Inventor/bundles/SoTextureCoordinateBundle.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoCoordinateElement.h>
#include <Inventor/elements/SoClipPlaneElement.h>
#include <Inventor/elements/SoTextureQualityElement.h>

#include <VolumeViz/elements/SoVolumeDataElement.h>
#include <VolumeViz/elements/SoTransferFunctionElement.h>
#include <VolumeViz/nodes/SoVolumeData.h>
#include <VolumeViz/misc/CvrCLUT.h>
#include <VolumeViz/misc/CvrVoxelChunk.h>
#include <VolumeViz/misc/CvrUtil.h>

#include "SoVolumeIndexedFaceSet.h"

// *************************************************************************

SO_NODE_SOURCE(SoVolumeIndexedFaceSet);

// *************************************************************************

class SoVolumeIndexedFaceSetP {
public:
  SoVolumeIndexedFaceSetP(SoVolumeIndexedFaceSet * master)
  {
    this->master = master;
    this->cube = NULL;
    this->clut = NULL;

    this->clipgeometryfaceset = new SoIndexedFaceSet;
    this->clipgeometryfaceset->ref();
    this->parentnodeid = master->getNodeId();
  }

  void setPalette(const CvrCLUT * c);

  Cvr3DTexCube * cube;
  const CvrCLUT * clut;
  SoIndexedFaceSet * clipgeometryfaceset;
  uint32_t parentnodeid;
  SbBool force2dtextures;

private:
  SoVolumeIndexedFaceSet * master;
};

#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

// *************************************************************************

SoVolumeIndexedFaceSet::SoVolumeIndexedFaceSet(void)
{
  SO_NODE_CONSTRUCTOR(SoVolumeIndexedFaceSet);
  PRIVATE(this) = new SoVolumeIndexedFaceSetP(this);

  SO_NODE_ADD_FIELD(clipGeometry, (FALSE));
  SO_NODE_ADD_FIELD(offset, (0.0f));

  // Shall we force 2D textures?
  const char * envstr = coin_getenv("CVR_FORCE_2D_TEXTURES");
  if (envstr) { PRIVATE(this)->force2dtextures = atoi(envstr) > 0 ? TRUE : FALSE; }
  else PRIVATE(this)->force2dtextures = 0;
 
}

SoVolumeIndexedFaceSet::~SoVolumeIndexedFaceSet(void)
{
  if (PRIVATE(this)->clut) { PRIVATE(this)->clut->unref(); }
  if (PRIVATE(this)->clipgeometryfaceset) { PRIVATE(this)->clipgeometryfaceset->unref(); }
  delete PRIVATE(this)->cube;
  delete PRIVATE(this);
}

// Doc from parent class.
void
SoVolumeIndexedFaceSet::initClass(void)
{
  SO_NODE_INIT_CLASS(SoVolumeIndexedFaceSet, SoIndexedFaceSet, "SoIndexedFaceSet");
}

void
SoVolumeIndexedFaceSetP::setPalette(const CvrCLUT * c)
{
  assert(c != NULL);

  if (this->clut) { this->clut->unref(); }
  this->clut = c;
  this->clut->ref();  
  this->cube->setPalette(c);
}


void
SoVolumeIndexedFaceSet::GLRender(SoGLRenderAction * action)
{

  // FIXME: need to make sure we're not cached in a renderlist
  if (!this->shouldGLRender(action)) return;

  // Render at the end, in case the volume is partly (or fully)
  // transparent.
  //
  // FIXME: this makes rendering a bit slower, so we should perhaps
  // keep a flag around to know whether or not this is actually
  // necessary. 20040212 mortene.
  
  if (!action->isRenderingDelayedPaths()) {
    action->addDelayedPath(action->getCurPath()->copy());
    return;
  }
  
  // FIXME: Support for 'offset' must be implemented. (20040628
  // handegar)
  if (this->offset.getValue() != 0) {
    static SbBool flag = FALSE;
    if (!flag) {
      SoDebugError::postWarning("SoVolumeIndexedFaceSet::GLRender", 
                                "Support for offset > 0 not implemented yet.");
      flag = TRUE;
    }
  }

  SoState * state = action->getState();

  const SoVolumeDataElement * volumedataelement = SoVolumeDataElement::getInstance(state);
  assert(volumedataelement != NULL);
  const SoVolumeData * volumedata = volumedataelement->getVolumeData();
  if (volumedata == NULL) { return; }

  // This must be done, as we want to control stuff in the GL state
  // machine. Without it, state changes could trigger outside our
  // control.
  state->push();

  SbMatrix volumetransform;
  CvrUtil::getTransformFromVolumeBoxDimensions(volumedataelement, volumetransform);
  SoModelMatrixElement::mult(state, this, volumetransform);
     
  SbVec3s dims = volumedataelement->getVoxelCubeDimensions();
  SbVec3f origo(-((float) dims[0]) / 2.0f, -((float) dims[1]) / 2.0f, -((float) dims[2]) / 2.0f);

  // Determiner rendering method.
  int rendermethod, storagehint; 
  rendermethod = SoVolumeIndexedFaceSet::TEXTURE2D; // this is the default
  storagehint = volumedata->storageHint.getValue();
  if (storagehint == SoVolumeData::TEX3D || storagehint == SoVolumeData::AUTO) {
    const cc_glglue * glue = cc_glglue_instance(action->getCacheContext());
    if (cc_glglue_has_3d_textures(glue) && !PRIVATE(this)->force2dtextures) {
      rendermethod = SoVolumeIndexedFaceSet::TEXTURE3D;
    }
  }
  
  if (rendermethod == SoVolumeIndexedFaceSet::TEXTURE2D) {    
    // 2D textures will not be supported for this node.
    static SbBool flag = FALSE;
    if (!flag) {
      SoDebugError::postWarning("SoVolumeIndexedFaceSet::GLRender", 
                                "2D textures not supported for this node.");
      flag = TRUE;
    }
  }
  else if (rendermethod == SoVolumeIndexedFaceSet::TEXTURE3D) {       
    
    // This must be done, as we want to control stuff in the GL state
    // machine. Without it, state changes could trigger outside our
    // control.
    SoGLLazyElement::getInstance(state)->send(state, SoLazyElement::ALL_MASK);
    
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glEnable(GL_TEXTURE_3D);
    glEnable(GL_DEPTH_TEST);
       
    if (!PRIVATE(this)->cube) { PRIVATE(this)->cube = new Cvr3DTexCube(volumedata->getReader()); }

    const SoTransferFunctionElement * tfelement = SoTransferFunctionElement::getInstance(state);
    const CvrCLUT * c = CvrVoxelChunk::getCLUT(tfelement);
    if (PRIVATE(this)->clut != c) { PRIVATE(this)->setPalette(c); }
    
    // Fetch texture quality
    float texturequality = SoTextureQualityElement::get(state);
    Cvr3DTexSubCube::Interpolation interp;
    if (texturequality >= 0.1f) interp = Cvr3DTexSubCube::LINEAR;
    else interp = Cvr3DTexSubCube::NEAREST;

    // Fetch vertices and normals from the stack    
    const SoCoordinateElement * coords;
    const SbVec3f * normals;
    const int32_t * cindices;
    int numindices;
    const int32_t * nindices;
    const int32_t * tindices;
    const int32_t * mindices;
    SbBool doTextures;
    SbBool normalCacheUsed;
    
    SoMaterialBundle mb(action);
    
    SoTextureCoordinateBundle tb(action, TRUE, FALSE);
    doTextures = tb.needCoordinates();
    SbBool sendNormals = !mb.isColorOnly() || tb.isFunction();
    
    this->getVertexData(state, coords, normals, cindices,
                        nindices, tindices, mindices, numindices,
                        sendNormals, normalCacheUsed);

    PRIVATE(this)->cube->renderIndexedFaceSet(action, origo, interp, 
                                              coords->getArrayPtr3(), 
                                              cindices, numindices); 
    glPopAttrib();

  }
  else { assert(FALSE && "Unknown rendering method."); }

  
  // Render the geometry which are outside the volume cube as polygons.
  if (this->clipGeometry.getValue()) {
           
    if (PRIVATE(this)->parentnodeid != this->getNodeId()) { // Changed recently?
      int i=0;
      for (i=0;i<this->coordIndex.getNum();++i)
        PRIVATE(this)->clipgeometryfaceset->coordIndex.set1Value(i, this->coordIndex[i]);
      for (i=0;i<this->materialIndex.getNum();++i)
        PRIVATE(this)->clipgeometryfaceset->materialIndex.set1Value(i, this->materialIndex[i]);
      for (i=0;i<this->normalIndex.getNum();++i)
        PRIVATE(this)->clipgeometryfaceset->normalIndex.set1Value(i, this->normalIndex[i]);
      // No need to copy texture coords as the face set shall always be untextured.
      PRIVATE(this)->parentnodeid = this->getNodeId();
    }
  
    SbPlane cubeplanes[6];
    SbVec3f a, b, c;
    
    // FIXME: Its really not needed to calculate the clip planes each
    // frame unless the volume has changed. This should be
    // optimized.(20040629 handegar)
    volumetransform.multVecMatrix(SbVec3f(origo + SbVec3f(0.0f, dims[1], 0.0f)), a);
    volumetransform.multVecMatrix(SbVec3f(origo + SbVec3f(0.0f, dims[1], dims[2])), b);
    volumetransform.multVecMatrix(SbVec3f(origo + SbVec3f(dims[0], dims[1], 0.0f)), c);    
    cubeplanes[0] = SbPlane(a, b, c); // Top     
    volumetransform.multVecMatrix(SbVec3f(origo), a);
    volumetransform.multVecMatrix(SbVec3f(origo + SbVec3f(dims[0], 0.0f, 0.0f)), b);
    volumetransform.multVecMatrix(SbVec3f(origo + SbVec3f(0.0f, 0.0f, dims[2])), c);    
    cubeplanes[1] = SbPlane(a, b, c); // Bottom    
    volumetransform.multVecMatrix(SbVec3f(origo), a);
    volumetransform.multVecMatrix(SbVec3f(origo + SbVec3f(dims[0], 0.0f, 0.0f)), b);
    volumetransform.multVecMatrix(SbVec3f(origo + SbVec3f(0.0f, 0.0f, dims[2])), c);    
    cubeplanes[2] = SbPlane(a, b, c); // Back    
    volumetransform.multVecMatrix(SbVec3f(origo + SbVec3f(0.0f, 0.0f, dims[2])), a);
    volumetransform.multVecMatrix(SbVec3f(origo + SbVec3f(dims[0], 0.0f, dims[2])), b);
    volumetransform.multVecMatrix(SbVec3f(origo + SbVec3f(0.0f, dims[1], dims[2])), c);    
    cubeplanes[3] = SbPlane(a, b, c); // Front    
    volumetransform.multVecMatrix(SbVec3f(origo + SbVec3f(dims[0], 0.0f, 0.0f)), a);
    volumetransform.multVecMatrix(SbVec3f(origo + SbVec3f(dims[0], dims[1], 0.0f)), b);
    volumetransform.multVecMatrix(SbVec3f(origo + SbVec3f(dims[0], 0.0f, dims[2])), c);    
    cubeplanes[4] = SbPlane(a, b, c); // Right    
    volumetransform.multVecMatrix(SbVec3f(origo), a);
    volumetransform.multVecMatrix(SbVec3f(origo + SbVec3f(0.0f, 0.0f, dims[2])), b);
    volumetransform.multVecMatrix(SbVec3f(origo + SbVec3f(0.0f, dims[1], 0.0f)), c);    
    cubeplanes[5] = SbPlane(a, b, c); // Left
    

    // 'un-Transform' model matrix before rendering clip geometry.
    state->pop();

    for (int i=0;i<6;++i) {
      state->push();       
      SoClipPlaneElement::add(state, this, cubeplanes[i]);    
      PRIVATE(this)->clipgeometryfaceset->GLRender(action);
      state->pop();
    }    
    return; // State is already pop'ed. Return.
  }

  state->pop();
}

void
SoVolumeIndexedFaceSet::rayPick(SoRayPickAction * action)
{
  // FIXME: Implement me? (20040628 handegar)
}

