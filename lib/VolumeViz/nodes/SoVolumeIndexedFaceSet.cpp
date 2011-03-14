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

/*!
  \class SoVolumeIndexedFaceSet VolumeViz/nodes/SoVolumeIndexedFaceSet.h
  \brief Render a set of faces within the volume.

  This node works like the SoVolumeFaceSet node, but specifies vertex
  indices in a slightly different manner. See documentation of
  SoVolumeFaceSet and Coin's SoIndexedFaceSet for further information.

  Note that this node will not work with OpenGL drivers too old to
  contain support for 3D-texturing. See the extended comments on
  SoObliqueSlice for more information.

  \sa SoVolumeFaceSet, SoVolumeRender, SoOrthoSlice, SoObliqueSlice
  \sa SoVolumeIndexedTriangleStripSet, SoVolumeTriangleStripSet

  \since SIM Voleon 2.0
*/

// *************************************************************************

#include <VolumeViz/nodes/SoVolumeIndexedFaceSet.h>

#include <Inventor/C/tidbits.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoGLLazyElement.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoCoordinateElement.h>
#include <Inventor/elements/SoClipPlaneElement.h>
#include <Inventor/elements/SoTextureQualityElement.h>
#include <Inventor/elements/SoShapeStyleElement.h>
#include <Inventor/elements/SoCacheElement.h>
#include <Inventor/nodes/SoMaterial.h>

#include <VolumeViz/nodes/SoVolumeData.h>
#include <VolumeViz/elements/CvrGLInterpolationElement.h>
#include <VolumeViz/elements/SoTransferFunctionElement.h>
#include <VolumeViz/elements/So2DTransferFunctionElement.h>
#include <VolumeViz/elements/CvrStorageHintElement.h>
#include <VolumeViz/elements/CvrVoxelBlockElement.h>
#include <VolumeViz/misc/CvrCLUT.h>
#include <VolumeViz/misc/CvrVoxelChunk.h>
#include <VolumeViz/misc/CvrUtil.h>
#include <VolumeViz/misc/CvrGlobalRenderLock.h>
#include <VolumeViz/render/raycast/CvrRaycastIndexedFaceSet.h>

#include "CvrIndexedFaceSetRenderP.h"

#include <RenderManager.h>


// *************************************************************************

SO_NODE_SOURCE(SoVolumeIndexedFaceSet);

// *************************************************************************

class SoVolumeIndexedFaceSetP {
public:
  SoVolumeIndexedFaceSetP(SoVolumeIndexedFaceSet * master)
  {
    this->master = master;   
    this->renderp = new CvrIndexedFaceSetRenderP(master);
    this->renderp->clipgeometryshape = new SoIndexedFaceSet;
    this->renderp->clipgeometryshape->ref();    

    this->raycastfaceset = NULL;

    this->raycastchromakey = new SoMaterial;
    this->raycastchromakey->diffuseColor.set1Value(0, SbColor(1.0, 0.0, 1.0));
    this->raycastchromakey->emissiveColor.set1Value(0, SbColor(1.0, 0.0, 1.0));
    this->raycastchromakey->ambientColor.set1Value(0, SbColor(1.0, 0.0, 1.0));
    this->raycastchromakey->shininess = 0.0f;
    this->raycastchromakey->transparency = 0.0f;
    this->raycastchromakey->ref();
  }
  ~SoVolumeIndexedFaceSetP() {
    this->renderp->clipgeometryshape->unref();
    delete this->renderp;
    this->raycastchromakey->unref();
  }
  CvrIndexedFaceSetRenderP * renderp;

  CvrRaycastIndexedFaceSet * raycastfaceset;
  SoMaterial * raycastchromakey;

private:
  SoVolumeIndexedFaceSet * master;
};

#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

// FIXME: Move this to a Util-class? (20101115 handegar)
namespace {
  // helper class used to push/pop the state
  class SoStatePushPop {    
  public:
    SoStatePushPop(SoState * state_in) : state(state_in) {
      state->push();
    }
    ~SoStatePushPop() {
      this->state->pop();
    }
  private:
    SoState * state;
  };
  
};


// *************************************************************************

SoVolumeIndexedFaceSet::SoVolumeIndexedFaceSet(void)
{
  SO_NODE_CONSTRUCTOR(SoVolumeIndexedFaceSet);
  PRIVATE(this) = new SoVolumeIndexedFaceSetP(this);
 
  SO_NODE_ADD_FIELD(clipGeometry, (FALSE));
  SO_NODE_ADD_FIELD(offset, (0.0f));
}

SoVolumeIndexedFaceSet::~SoVolumeIndexedFaceSet(void)
{
  delete PRIVATE(this); 
}

// Doc from parent class.
void
SoVolumeIndexedFaceSet::initClass(void)
{
  SO_NODE_INIT_CLASS(SoVolumeIndexedFaceSet, SoIndexedFaceSet, "SoIndexedFaceSet");

  SO_ENABLE(SoGLRenderAction, CvrGLInterpolationElement);
}

void
SoVolumeIndexedFaceSet::GLRender(SoGLRenderAction * action)
{
  // This will automatically lock and unlock a mutex stopping multiple
  // render threads for SIM Voleon nodes. FIXME: should really make
  // code re-entrant / threadsafe. 20041112 mortene.
  CvrGlobalRenderLock lock;

  SoState * state = action->getState();
  SoStatePushPop pushpop(state);

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



  const int storagehint = CvrStorageHintElement::get(state);
  // Do a quick test to see if OpenCL rendering is supported
  if (storagehint == SoVolumeData::RAYCAST) {
    static SbBool checked = FALSE;
    static SbBool supports = FALSE;
    if (!checked) {
      static SbBool first = TRUE;   
      supports = CLVol::RenderManager::supportsOpenCL();
      if (first && !supports) {
        SoDebugError::postWarning("SoVolumeIndexedFaceSet::GLRender",
                                  "Warning: System does not support OpenCL "
                                  "raycast rendering. -- Rendering aborted.");
        first = false;    
      }
      checked = TRUE;
    }    
    if (!supports) {
      assert(0);
      return; // OpenCL not supported.
    }

    // We'll have to invalidate the cache or something will start
    // reporting GL_ERRORS in Coin due to displaylist errors.
    SoCacheElement::invalidate(state);

    if (!PRIVATE(this)->raycastfaceset) {
      PRIVATE(this)->raycastfaceset = new CvrRaycastIndexedFaceSet(action);     
    }
    
    const uint32_t glctxid = action->getCacheContext();
    const cc_glglue * glw = cc_glglue_instance(glctxid);
    cc_glglue_glBindFramebuffer(glw, GL_FRAMEBUFFER, 0);


    // Apply chromakey color/material.
    action->traverse(PRIVATE(this)->raycastchromakey);

    inherited::GLRender(action); 
    
    const CvrVoxelBlockElement * vbelement = CvrVoxelBlockElement::getInstance(state);
    SbMatrix volumetransform;
    CvrUtil::getTransformFromVolumeBoxDimensions(vbelement, volumetransform);
    SoModelMatrixElement::mult(state, this, volumetransform);
    
    // FIXME: Detect changes before setting the transferfunction
    // (20100916 handegar)
    const So2DTransferFunctionElement * tfe = 
      So2DTransferFunctionElement::getInstance(state);
    PRIVATE(this)->raycastfaceset->setTransferFunction(tfe->getTransferFunction());      
    PRIVATE(this)->raycastfaceset->render(action, this);
    
  }
  else {        
    PRIVATE(this)->renderp->GLRender(action, this->offset.getValue(), 
                                     this->clipGeometry.getValue());
  }

}
