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

/*!
  \class SoObliqueSlice VolumeViz/nodes/SoObliqueSlice.h
  \brief Render a slice of the volume data, with any orientation.

  Insert a node of this type after an SoVolumeData node in the scene
  graph to render a single slice from the full volume data set. The
  slice is specified as a plane with an orientation and position
  within the volume.

  \image html vol-obliqueslice.png "Rendering of an oblique slice"

  Here is a simple example, in the form of an iv-file:

  \verbatim
  #Inventor V2.1 ascii

  SoVolumeData { fileName "ENGINE.VOL" }
   
  Separator {
    SoTransferFunction { }  
    DEF clipper ClipPlaneManip { }
    SoVolumeRender { }   
  }

  Separator {
    SoTransferFunction { predefColorMap GLOW }  
    SoObliqueSlice {
      interpolation LINEAR 
      alphaUse ALPHA_AS_IS
      plane = USE clipper . plane
    }
  }
  \endverbatim

  Please note that SoObliqueSlice will not work with older OpenGL
  drivers as they usually lack 3D-texture support. OpenGL drivers of
  version 1.2 and onwards supports 3D-texturing, as does older OpenGL
  drivers with the \c GL_EXT_texture3D extension. If none of these are
  available, a warning message will be printed using Coin's
  SoDebugError::postWarning().

  \sa SoVolumeRender, SoOrthoSlice
  \sa SoVolumeTriangleStripSet, SoVolumeIndexedTriangleStripSet, 
  \sa SoVolumeIndexedFaceSet, SoVolumeFaceSet 

  \since SIM Voleon 2.0
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
#include <VolumeViz/misc/CvrGlobalRenderLock.h>

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

  // FIXME: implement proper support for alternateRep field. 20041008 mortene.
  SO_NODE_ADD_FIELD(alternateRep, (NULL));
}

SoObliqueSlice::~SoObliqueSlice()
{
  delete PRIVATE(this)->cubehandler;
  delete PRIVATE(this);
}

// *************************************************************************

/*!
  \enum SoObliqueSlice::Interpolation
  Enumeration of available types of voxel color interpolation.
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

  All alpha values not equal to 0 are treated as value 1.0f.
*/

/*!
  \var SoSFEnum SoObliqueSlice::alphaUse

  How the alpha channel should be treated during rendering.
  See SoObliqueSlice::AlphaUse.
  
  Default value is SoObliqueSlice::ALPHA_AS_IS
*/


/*!
  \var SoSFPlane SoObliqueSlice::plane

  The plane defining the slice.
  
  Default value is an XY plane.
*/

/*!
  \var SoSFNode SoObliqueSlice::alternateRep
  NOTE: support for this field not yet implemented in SIM Voleon.
  \since SIM Voleon 2.0
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
                                    "obsolete CVR_USE_FLIPPED_Y_AXIS envvar is active.");
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

// *************************************************************************

// Overridden so we can give special attention to alternateRep.
void
SoObliqueSlice::write(SoWriteAction * action)
{
  // FIXME: implement proper support for alternateRep field. 20041008 mortene.
  if (this->alternateRep.getValue() != NULL) {
    SoDebugError::postWarning("SoObliqueSlice::write",
                              "no support for alternateRep field yet, "
                              "alternate geometry ignored");
  }

  inherited::write(action);
}

// Overridden so we can give special attention to alternateRep.
SbBool
SoObliqueSlice::readInstance(SoInput * in, unsigned short flags)
{
  const SbBool ret = inherited::readInstance(in, flags);

  // FIXME: implement proper support for alternateRep field. 20041008 mortene.
  if (this->alternateRep.getValue() != NULL) {
    SoDebugError::postWarning("SoObliqueSlice::readInstance",
                              "no support for alternateRep field yet, "
                              "alternate geometry ignored");
  }

  return ret;
}

// *************************************************************************
