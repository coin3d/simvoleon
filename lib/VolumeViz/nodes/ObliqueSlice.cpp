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

/*!
  \class SoObliqueSlice VolumeViz/nodes/SoObliqueSlice.h
  \brief Render a slice of the volume data, with any orientation.

  Insert a node of this type after an SoVolumeData node in the scene
  graph to render a single slice from the full volume data set. The
  slice is specified as a plane with any orientation and position
  within the volume.

  <center>
   <img src="http://doc.coin3d.org/images/SIMVoleon/nodes/vol-obliqueslice.png">
  </center>

  Note that this node will not work with OpenGL drivers too old to
  contain support for 3D-texturing. All OpenGL drivers of version 1.2
  and onwards supports 3D-texturing, as does also older OpenGL drivers
  with the \c GL_EXT_texture3D extension. If none of these are
  available, and this node is still attempted used, a warning message
  will be output through Coin's SoDebugError::postWarning().

  \sa SoVolumeRender, SoOrthoSlice
  \sa SoVolumeTriangleStripSet, SoVolumeIndexedTriangleStripSet, 
  \sa SoVolumeIndexedFaceSet, SoVolumeFaceSet 

  \since SIM Voleon 1.1
*/

// *************************************************************************

#include <VolumeViz/nodes/SoObliqueSlice.h>

#include <Inventor/C/glue/gl.h>
#include <Inventor/C/tidbits.h>
#include <Inventor/SbBox3s.h>
#include <Inventor/SbRotation.h>
#include <Inventor/SoPickedPoint.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoRayPickAction.h>
#include <Inventor/elements/SoGLLazyElement.h>
#include <Inventor/elements/SoLazyElement.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/errors/SoDebugError.h>

#include <VolumeViz/details/SoObliqueSliceDetail.h>
#include <VolumeViz/elements/CvrGLInterpolationElement.h>
#include <VolumeViz/elements/CvrStorageHintElement.h>
#include <VolumeViz/elements/CvrVoxelBlockElement.h>
#include <VolumeViz/elements/SoTransferFunctionElement.h>
#include <VolumeViz/render/3D/CvrCubeHandler.h>
#include <VolumeViz/misc/CvrUtil.h>

// *************************************************************************

SO_NODE_SOURCE(SoObliqueSlice);

// *************************************************************************

class SoObliqueSliceP {
public:
  SoObliqueSliceP(SoObliqueSlice * master)
  {
    this->master = master;
    this->cubehandler = NULL;
  }

  CvrCubeHandler * cubehandler;

private:
  SoObliqueSlice * master;
};

#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

// *************************************************************************

SoObliqueSlice::SoObliqueSlice(void)
{
  SO_NODE_CONSTRUCTOR(SoObliqueSlice);

  PRIVATE(this) = new SoObliqueSliceP(this);

  SO_NODE_DEFINE_ENUM_VALUE(Interpolation, NEAREST);
  SO_NODE_DEFINE_ENUM_VALUE(Interpolation, LINEAR);
  SO_NODE_SET_SF_ENUM_TYPE(interpolation, Interpolation);

  SO_NODE_DEFINE_ENUM_VALUE(AlphaUse, ALPHA_AS_IS);
  SO_NODE_DEFINE_ENUM_VALUE(AlphaUse, ALPHA_OPAQUE);
  SO_NODE_DEFINE_ENUM_VALUE(AlphaUse, ALPHA_BINARY);
  SO_NODE_SET_SF_ENUM_TYPE(alphaUse, AlphaUse);

  SO_NODE_ADD_FIELD(plane, (SbPlane(SbVec3f(0, 0, 1), 0)));
  SO_NODE_ADD_FIELD(interpolation, (LINEAR));
  SO_NODE_ADD_FIELD(alphaUse, (ALPHA_BINARY));
}

SoObliqueSlice::~SoObliqueSlice()
{
  delete PRIVATE(this)->cubehandler;
  delete PRIVATE(this);
}

// *************************************************************************

/*!
  \enum SoObliqueSlice::Interpolation
  Enumeration of available types of voxel colors interpolation.
*/
/*!
  \var SoObliqueSlice::Interpolation SoObliqueSlice::NEAREST

  For "in between" pixels of the screen rasterization, pick the color
  of the nearest voxel.  Will give sharp edges and a distinct blocky
  look.
*/
/*!
  \var SoObliqueSlice::Interpolation SoObliqueSlice::LINEAR

  For "in between" pixels of the screen rasterization, interpolate by
  averaging the colors of several of the nearest voxels.  Will give a
  smoother appearance, but sacrifies some "correctness" for
  appearance.
*/
/*!
  \var SoSFEnum SoObliqueSlice::interpolation

  How to interpolate color values when rendering "in between" voxels.
  See SoObliqueSlice::Interpolation.

  Default value is SoObliqueSlice::LINEAR.
*/


/*!
  \var SoObliqueSlice::alphaUse SoObliqueSlice::ALPHA_AS_IS
  
  The alpha channel is rendered 'as-is'.
*/
/*!
  \var SoObliqueSlice::alphaUse SoObliqueSlice::ALPHA_OPAQUE

  The alpha channel is ignored making the slice opaque.
*/
/*!
  \var SoObliqueSlice::alphaUse SoObliqueSlice::ALPHA_BINARY

  All alpha values not equal to 0 is threated as value 1.0f.
*/

/*!
  \var SoSFEnum SoObliqueSlice::alphaUse

  How the alpha channel should be threated during rendering.
  See SoObliqueSlice::AlphaUse.
  
  Default value is SoObliqueSlice::ALPHA_AS_IS
*/


/*!
  \var SoSFPlane SoObliqueSlice::plane

  The plane defining the slice.
  
  Default value is an XY plane.
*/

// *************************************************************************


// Doc from parent class.
void
SoObliqueSlice::initClass(void)
{
  SO_NODE_INIT_CLASS(SoObliqueSlice, SoShape, "SoShape");

  SO_ENABLE(SoGLRenderAction, SoTransferFunctionElement);
  SO_ENABLE(SoGLRenderAction, SoLazyElement);

  SO_ENABLE(SoRayPickAction, SoTransferFunctionElement);

  SO_ENABLE(SoGLRenderAction, CvrGLInterpolationElement);
}

void
SoObliqueSlice::GLRender(SoGLRenderAction * action)
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

  SoState * state = action->getState();
   
  // Fetching the current volumedata
  const CvrVoxelBlockElement * vbelem = CvrVoxelBlockElement::getInstance(state);
  if (vbelem == NULL) {
    static SbBool first = TRUE;
    if (first) {
      SoDebugError::post("SoObliqueSlice::GLRender",
                         "no SoVolumeData in scene graph before "
                         "SoObliqueSlice node -- rendering aborted");
      first = FALSE;
    }
    return;
  }

  // Fetching the current transfer function. Note that it's not used
  // in this function, but we still catch this exception here for the
  // sake of simplicity of the code we're calling.
  const SoTransferFunctionElement * transferfunctionelement =
    SoTransferFunctionElement::getInstance(state);
  assert(transferfunctionelement != NULL);
  
  SoTransferFunction * transferfunction =
    transferfunctionelement->getTransferFunction();

   
  if (transferfunction == NULL) {
    // FIXME: should instead just use a default
    // transferfunction. Perhaps SoVolumeData (?) could place one the
    // state stack? 20040220 mortene.
    static SbBool first = TRUE;
    if (first) {
      SoDebugError::post("SoObliqueSlice::GLRender",
                         "no SoTransferFunction in scene graph before "
                         "SoObliqueSlice node -- rendering aborted");
      first = FALSE;
    }
    return;
  }

  // This must be done, as we want to control stuff in the GL state
  // machine. Without it, state changes could trigger outside our
  // control.
  state->push();

  SbMatrix volumetransform;
  CvrUtil::getTransformFromVolumeBoxDimensions(vbelem, volumetransform);
  SoModelMatrixElement::mult(state, this, volumetransform);

  const SbVec3s & voxcubedims = vbelem->getVoxelCubeDimensions();

  const cc_glglue * glue = cc_glglue_instance(action->getCacheContext());
  if (!cc_glglue_has_3d_textures(glue)) {
    static SbBool flag = FALSE;
    if (!flag) {
      SoDebugError::postWarning("SoObliqueSlice::GLRender", 
                                "Your OpenGL driver does not support 3D "
                                "textures, which is needed for rendering the "
                                "SoObliqueSlice.");
      flag = TRUE;
    }
    goto done;
  }

     
  if (!PRIVATE(this)->cubehandler) {
    PRIVATE(this)->cubehandler = new CvrCubeHandler();
  }

  GLenum interp;
  switch (this->interpolation.getValue()) {
  case NEAREST: interp = GL_NEAREST; break;
  case LINEAR: interp = GL_LINEAR; break;
  default: assert(FALSE && "invalid value in interpolation field"); break;
  }
  CvrGLInterpolationElement::set(state, interp);
    
  SoObliqueSlice::AlphaUse alphause;    
  switch (this->alphaUse.getValue()) {
  case ALPHA_AS_IS: alphause = SoObliqueSlice::ALPHA_AS_IS; break;
  case ALPHA_OPAQUE: alphause = SoObliqueSlice::ALPHA_OPAQUE; break;
  case ALPHA_BINARY: alphause = SoObliqueSlice::ALPHA_BINARY; break;
  default: assert(FALSE && "invalid value in AlphaUse field"); break;
  }
    
  PRIVATE(this)->cubehandler->renderObliqueSlice(action, alphause, this->plane.getValue());

 done:
  state->pop();
}

void
SoObliqueSlice::rayPick(SoRayPickAction * action)
{
  if (!this->shouldRayPick(action)) return;

  SoState * state = action->getState();
  const CvrVoxelBlockElement * vbelem = CvrVoxelBlockElement::getInstance(state);
  if (vbelem == NULL) { return; }

  this->computeObjectSpaceRay(action);
  const SbLine & ray = action->getLine();

  const SbPlane sliceplane = this->plane.getValue();

  SbVec3f intersection;
  if (sliceplane.intersect(ray, intersection) && // returns FALSE if parallel
      action->isBetweenPlanes(intersection)) {

    SbVec3s ijk = vbelem->objectCoordsToIJK(intersection);

    const SbVec3s & voxcubedims = vbelem->getVoxelCubeDimensions();
    const SbBox3s voxcubebounds(SbVec3s(0, 0, 0), voxcubedims - SbVec3s(1, 1, 1));

    if (voxcubebounds.intersect(ijk)) {

      SoPickedPoint * pp = action->addIntersection(intersection);
      // if NULL, something else is obstructing the view to the
      // volume, the app programmer only want the nearest, and we
      // don't need to continue our intersection tests
      if (pp == NULL) return;
      pp->setObjectNormal(sliceplane.getNormal());

      SoObliqueSliceDetail * detail = new SoObliqueSliceDetail;
      pp->setDetail(detail, this);

      detail->objectcoords = intersection;
      detail->ijkcoords = ijk;
      detail->voxelvalue = vbelem->getVoxelValue(ijk);

      if (CvrUtil::useFlippedYAxis()) {
        static SbBool flag = FALSE;
        if (!flag) {
          SoDebugError::postWarning("SoObliqueSlice::rayPick", 
                                    "RayPick'ing will not be correct for SoObliqueSlice when the "
                                    "obsolete CVR_USE_FLIPPED_Y_AXIS flag is active.");
          flag = TRUE;
        }
      }

    }
  }

  // Common clipping plane handling.
  SoObliqueSlice::doAction(action);
}

void
SoObliqueSlice::generatePrimitives(SoAction * action)
{
  // FIXME: implement me?

#if CVR_DEBUG && 1 // debug
  static SbBool warn = TRUE;
  if (warn) {
    SoDebugError::postInfo("SoObliqueSlice::generatePrimitives",
                           "not yet implemented");
    warn = FALSE;
  }
#endif // debug
}

// doc in super
void
SoObliqueSlice::computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center)
{
  SoState * state = action->getState();
  const CvrVoxelBlockElement * vbelem = CvrVoxelBlockElement::getInstance(state);
  if (vbelem == NULL) return;

  const SbBox3f & vdbox = vbelem->getUnitDimensionsBox();

  box.extendBy(vdbox);
  center = vdbox.getCenter();
}
