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
  \class SoVolumeSkin VolumeViz/nodes/SoVolumeSkin.h
  \brief Render just the six sides of the volume data.

  Insert a node of this type after an SoVolumeData node in the scene
  graph to render the six external sides of the volume data set.

  This can be useful as a very efficient way of visualizing a complete
  opaque volume, only seen from the outside.

  Note that if the transfer function set by a preceding
  SoTransferFunction node contains transparency, the visual appearance
  to the end user will probably not look correct.

  \sa SoVolumeRender, SoTransferFunction, SoOrthoSlice

  \since SIM Voleon 2.0
*/

// *************************************************************************


#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/SbLinear.h>
#include <Inventor/actions/SoRayPickAction.h>

#include <VolumeViz/elements/CvrVoxelBlockElement.h>
#include <VolumeViz/nodes/SoOrthoSlice.h>
#include <VolumeViz/nodes/SoVolumeSkin.h>
#include <VolumeViz/details/SoVolumeSkinDetail.h>
#include <VolumeViz/misc/CvrCLUT.h>
#include <VolumeViz/misc/CvrUtil.h>
#include <VolumeViz/misc/CvrGlobalRenderLock.h>

#include "volumeraypickintersection.h"

// *************************************************************************

SO_NODE_SOURCE(SoVolumeSkin);

// *************************************************************************

struct slicestruct {
  SoOrthoSlice * slice;
  float distance;
};

class SoVolumeSkinP {
public:
  SoVolumeSkinP(SoVolumeSkin * master)
  {
    this->master = master;
    this->initialized = FALSE;
  }
  ~SoVolumeSkinP()
  {
    if (this->initialized) {
      for (int i=0;i<6;++i) 
        this->cubesides[i].slice->unref();    
      delete this->cubesides;
    }
  }

  void buildSkinCube(SoGLRenderAction * action);
  void sortCubeSides(SoGLRenderAction * action);
  void renderSkinCube(SoGLRenderAction * action, 
                      enum SoVolumeSkin::Interpolation interpolation);

  slicestruct * cubesides;
  SoVolumeSkin::Interpolation interpolation;
  SbBool initialized;

private:
  SoVolumeSkin * master;
};

#define PRIVATE(p) (p->pimpl)

// *************************************************************************

SoVolumeSkin::SoVolumeSkin(void)
{
  SO_NODE_CONSTRUCTOR(SoVolumeSkin);

  PRIVATE(this) = new SoVolumeSkinP(this);

  SO_NODE_DEFINE_ENUM_VALUE(Interpolation, NEAREST);
  SO_NODE_DEFINE_ENUM_VALUE(Interpolation, LINEAR);
  SO_NODE_SET_SF_ENUM_TYPE(interpolation, Interpolation);
  SO_NODE_ADD_FIELD(interpolation, (LINEAR));
}

SoVolumeSkin::~SoVolumeSkin()
{
  delete PRIVATE(this);
}

// Doc from parent class.
void
SoVolumeSkin::initClass(void)
{
  SO_NODE_INIT_CLASS(SoVolumeSkin, SoShape, "SoShape");
}

void
SoVolumeSkinP::buildSkinCube(SoGLRenderAction * action)
{

  this->cubesides = new slicestruct[6];

  for (int i=0;i<6;++i) {
    this->cubesides[i].slice = new SoOrthoSlice;
    this->cubesides[i].slice->ref();
    this->cubesides[i].distance = 0.0f;
  }

  const CvrVoxelBlockElement * vbelem =
    CvrVoxelBlockElement::getInstance(action->getState());
  assert(vbelem != NULL);
  const SbVec3s & voxcubedims = vbelem->getVoxelCubeDimensions();

  const int maxslicesx = voxcubedims[0]-1;
  const int maxslicesy = voxcubedims[1]-1;
  const int maxslicesz = voxcubedims[2]-1;

  // Top
  this->cubesides[0].slice->axis.setValue(SoOrthoSlice::Y);
  this->cubesides[0].slice->sliceNumber.setValue(maxslicesy);
  // Bottom
  this->cubesides[1].slice->axis.setValue(SoOrthoSlice::Y);
  this->cubesides[1].slice->sliceNumber.setValue(0);
  // Right
  this->cubesides[2].slice->axis.setValue(SoOrthoSlice::X);
  this->cubesides[2].slice->sliceNumber.setValue(maxslicesx);
  // Left
  this->cubesides[3].slice->axis.setValue(SoOrthoSlice::X);
  this->cubesides[3].slice->sliceNumber.setValue(0);  
  // Front
  this->cubesides[4].slice->axis.setValue(SoOrthoSlice::Z);
  this->cubesides[4].slice->sliceNumber.setValue(maxslicesz);
  // Back
  this->cubesides[5].slice->axis.setValue(SoOrthoSlice::Z);
  this->cubesides[5].slice->sliceNumber.setValue(0);
  
}


static int
qsort_compare(const void * element1, const void * element2)
{
  const slicestruct * s1 = (slicestruct *) element1;
  const slicestruct * s2 = (slicestruct *) element2;  
  if (s1->distance < s2->distance) return -1;
  else return 1;
}

void
SoVolumeSkinP::sortCubeSides(SoGLRenderAction * action)
{

  const SbViewVolume viewvolume = SoViewVolumeElement::get(action->getState());
  const SbPlane camplane = viewvolume.getPlane(0.0f);

  // Fetch camera distance to each slice
  for (int i=0;i<6;++i) {
    SbVec3f center;
    SbBox3f dummy;
    ((SoShape *) this->cubesides[i].slice)->computeBBox(action, dummy, center);
    this->cubesides[i].distance = camplane.getDistance(center);
  }
  
  // Sort rendering order
  qsort((void *) this->cubesides, 6, sizeof(slicestruct), qsort_compare);
  
}

void
SoVolumeSkinP::renderSkinCube(SoGLRenderAction * action, 
                              enum SoVolumeSkin::Interpolation interpolation)
{
  
  if (this->interpolation != interpolation) {
    for (int i=0;i<6;++i) 
      this->cubesides[i].slice->interpolation = interpolation;
    this->interpolation = interpolation;
  }
  
  this->sortCubeSides(action);
  
  for (int i=0;i<6;++i) 
    ((SoShape *) this->cubesides[i].slice)->GLRender(action);

}

void
SoVolumeSkin::GLRender(SoGLRenderAction * action)
{
  // This will automatically lock and unlock a mutex stopping multiple
  // render threads for SIM Voleon nodes. FIXME: should really make
  // code re-entrant / threadsafe. 20041112 mortene.
  CvrGlobalRenderLock lock;


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
  
  if (!PRIVATE(this)->initialized) {
    PRIVATE(this)->buildSkinCube(action);
    PRIVATE(this)->initialized = TRUE;
  }

  Interpolation interp;
  if (this->interpolation.getValue() == NEAREST) interp = NEAREST;
  else interp = LINEAR;

  PRIVATE(this)->renderSkinCube(action, interp);

}



void
SoVolumeSkin::generatePrimitives(SoAction * action)
{
  // FIXME: implement me?
#if CVR_DEBUG && 1 // debug
  static SbBool warn = TRUE;
  if (warn) {
    SoDebugError::postInfo("SoVolumeSkin::generatePrimitives",
                           "not yet implemented");
    warn = FALSE;
  }
#endif // debug
}


// doc in super
void
SoVolumeSkin::computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center)
{
  SoState * state = action->getState();

  const CvrVoxelBlockElement * vbelem = CvrVoxelBlockElement::getInstance(state);
  if (vbelem == NULL) { return; }

  const SbBox3f & vdbox = vbelem->getUnitDimensionsBox();
  if (vdbox.isEmpty()) { return; }

  box.extendBy(vdbox);
  center = vdbox.getCenter();
}


void
SoVolumeSkin::rayPick(SoRayPickAction * action)
{
 
  if (!this->shouldRayPick(action)) return;
  
  SbVec3f intersects[2];  
  SoState * state = action->getState();  
  this->computeObjectSpaceRay(action);
  
  if (!cvr_volumeraypickintersection(action, intersects))
    return;
    
  SoVolumeSkinDetail * detail = new SoVolumeSkinDetail;
  detail->setDetails(intersects[0], intersects[1], state, this);

}

// *************************************************************************

#undef PRIVATE
#undef PUBLIC
