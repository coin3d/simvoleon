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
  \class SoVolumeRender VolumeViz/nodes/SoVolumeRender.h
  \brief Render the full volume.

  Insert a node of this type after an SoVolumeData node in the scene
  graph to render the full volume data set.

  \sa SoOrthoSlice, SoObliqueSlice, SoVolumeFaceSet, SoVolumeIndexedFaceSet
  \sa SoVolumeTriangleStripSet, SoVolumeIndexedTriangleStripSet
*/

// *************************************************************************

#include <VolumeViz/nodes/SoVolumeRender.h>

#include <string.h>
#include <limits.h> // UINT_MAX
#include <float.h> // DBL_MAX

#include <Inventor/C/glue/gl.h>
#include <Inventor/C/tidbits.h>
#include <Inventor/SbLine.h>
#include <Inventor/SbPlane.h>
#include <Inventor/SbRotation.h>
#include <Inventor/SbTime.h>
#include <Inventor/SoPickedPoint.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/actions/SoRayPickAction.h>
#include <Inventor/bundles/SoMaterialBundle.h>
#include <Inventor/elements/SoGLTexture3EnabledElement.h>
#include <Inventor/elements/SoGLTextureEnabledElement.h>
#include <Inventor/elements/SoLazyElement.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/system/gl.h>
#include <Inventor/nodes/SoDrawStyle.h>
#include <Inventor/nodes/SoCube.h>

#include <VolumeViz/nodes/SoVolumeData.h>
#include <VolumeViz/elements/CvrGLInterpolationElement.h>
#include <VolumeViz/elements/CvrVoxelBlockElement.h>
#include <VolumeViz/elements/CvrStorageHintElement.h>
#include <VolumeViz/elements/SoTransferFunctionElement.h>
#include <VolumeViz/render/2D/CvrPageHandler.h>
#include <VolumeViz/render/3D/CvrCubeHandler.h>
#include <VolumeViz/render/Pointset/PointRendering.h>
#include <VolumeViz/details/SoVolumeRenderDetail.h>
#include <VolumeViz/misc/CvrVoxelChunk.h>
#include <VolumeViz/misc/CvrCLUT.h>
#include <VolumeViz/misc/CvrUtil.h>
#include <VolumeViz/misc/CvrGlobalRenderLock.h>

#include "volumeraypickintersection.h"

// *************************************************************************

// FIXME: not so sure the below whitelist and blacklist is such a good
// idea after all. There seems to be a realistic possibility for false
// positives or negatives.
//
// Should at least cut them down to these cards we have had hands-on
// experience with. Should also extend the driver identification
// strings to be more exacting, to minimize the potential for false
// positives and negatives.
//
// 20040708 mortene.

// Whitelist of GL_RENDERER substrings for cards which does 3D
// textures in hardware.
static const char * texture3d_in_hardware[] = {
#if 0 // tmp disabled, see above FIXME
  "GeForce FX",    // 5200 .. 5950
  "GeForce3",
  "GeForce4 Ti",
  "Geforce4 4200 GO",
  "Quadro FX",
  "Quadro4",
  "Quadro DCC",    // Has the nFiniteFX engine, so we assume it has HW 3D textures.
  "RADEON ",       // 7000 .. 9000
  "Radeon ",
  "RV250",         // A semi Radeon 8500
  "MOBILITY RADEON ",
  "Fire GL",       // I persume these highend cards do 3D textures in HW
  "Wildcat VP",    // 560, 870, 970.
#endif // tmp disabled
  NULL
};

// Blacklist of GL_RENDERER substrings for hardware which does 3D
// textures, but only in software (i.e. not feasible for volume
// rendering).
static const char * texture3d_in_software[] = {
#if 0 // tmp disabled, see above FIXME
  "GeForce2",
  "GeForce4 MX",
  "Geforce4 440 GO",
  "Geforce4 460 GO",
  "Geforce4 420 GO",
  "RAGE 128",
  "Mesa Windows",
#endif // tmp disabled
  NULL
};

// number of successive test times number of triangels to render each test.
static const unsigned int VOLUMERENDER_PERFORMANCETEST_TIMES = 10;
static const unsigned int VOLUMERENDER_PERFORMANCETEST_TRIANGLES = 5;

// *************************************************************************

SO_NODE_SOURCE(SoVolumeRender);

// *************************************************************************

class SoVolumeRenderP {
public:
  SoVolumeRenderP(SoVolumeRender * master)
  {
    this->master = master;
    this->pagehandler = NULL;
    this->cubehandler = NULL;
    this->abortfunc = NULL;
    this->abortfuncdata = NULL;
    this->linestylevolumecube = NULL;
  }

  ~SoVolumeRenderP()
  {
    if (this->linestylevolumecube) this->linestylevolumecube->unref();
    if (this->pagehandler) delete this->pagehandler;
    if (this->cubehandler) delete this->cubehandler;
  }

  unsigned int calculateNrOf2DSlices(SoGLRenderAction * action, const SbVec3s & dimensions);
  unsigned int calculateNrOf3DSlices(SoGLRenderAction * action, const SbVec3s & dimensions);
  SbBool use3DTexturing(const cc_glglue * glglue) const;

  static SbBool debug3DTextureTiming(void);
  static void setupPerformanceTestTextures(GLuint * texture3did,
                                           GLuint * texture2dids,
                                           const cc_glglue * glglue);
  static double performanceTest(const cc_glglue * glue);
  static void renderPerformanceTestScene(SbList<double> & timelist3d,
                                         SbList<double> & timelist2d,
                                         GLuint * texture3did,
                                         GLuint * texture2dids);
  static double getAveragePerformanceTime(SbList<double> & l);

  CvrPageHandler * pagehandler; // For 2D page rendering
  CvrCubeHandler * cubehandler; // For 3D cube rendering

  SoVolumeRender::SoVolumeRenderAbortCB * abortfunc;
  void * abortfuncdata;

  // A cube used as line-style rep. for the volume
  SoCube * linestylevolumecube;
  
  // debug
  void rayPickDebug(SoGLRenderAction * action);
  SbList<SbVec3f> raypicklines;

  enum RenderingMethod { TEXTURE3D, TEXTURE2D, NOTIMPLEMENTED };

private:
  SoVolumeRender * master;
};

#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

SbBool
SoVolumeRenderP::debug3DTextureTiming(void)
{
  // Debug envvar used for keeping the 3D and 2D textured polygons
  // visibly in the buffer.
  static int keeptestgfx = -1;
  if (keeptestgfx == -1) {
    const char * env = coin_getenv("CVR_DEBUG_3DTEX_PERFORMANCE");
    keeptestgfx = env && (atoi(env) > 0);
  }
  return keeptestgfx ? TRUE : FALSE;
}

// *************************************************************************

/*!
  \enum SoVolumeRender::Interpolation
  Enumeration of available types of voxel colors interpolation.
*/
/*!
  \var SoVolumeRender::Interpolation SoVolumeRender::NEAREST

  For "in between" pixels of the screen rasterization, pick the color
  of the nearest voxel.  Will give sharp edges and a distinct blocky
  look.
*/
/*!
  \var SoVolumeRender::Interpolation SoVolumeRender::LINEAR

  For "in between" pixels of the screen rasterization, interpolate by
  averaging the colors of several of the nearest voxels.  Will give a
  smoother appearance, but sacrifies some "correctness" for
  appearance.
*/

/*!
  \var SoSFEnum SoVolumeRender::interpolation

  How to interpolate color values when rendering "in between" voxels.
  See SoVolumeRender::Interpolation.

  Default value is SoVolumeRender::LINEAR.
*/


// *************************************************************************

/*!
  \enum SoVolumeRender::NumSlicesControl

  Enumeration of strategies for how to render the slices of the
  volume.

*/
/*!
  \var SoVolumeRender::NumSlicesControl SoVolumeRender::ALL

  Always render as many slices as there are voxels in the depth
  dimension. This is the default value.

  Please note that SoVolumeRender::NumSlicesControl will always be
  considered as SoVolumeRender::ALL if the SoVolumeRender::numSlices
  field is less or equal to 0.
*/
/*!
  \var SoVolumeRender::NumSlicesControl SoVolumeRender::MANUAL

  Render as many slices as given by the SoVolumeRender::numSlices
  field.
*/
/*!
  \var SoVolumeRender::NumSlicesControl SoVolumeRender::AUTOMATIC

  The number of slices to render will be calculated as follows:

  \code
  unsigned int numslices = complexity * 2.0f * this->numSlices;
  \endcode

  Where \a "complexity" is the current SoComplexity::value setting in
  the scene graph traversal state. The default complexity value for a
  scene graph with no SoComplexity node(s) is 0.5.

  For \a "this->numSlices", see SoVolumeRender::numSlices.
*/

/*!
  \var SoSFEnum SoVolumeRender::numSlicesControl

  Specifies a strategy to use for calculating the number of slices to
  use for the visualization. The more slicer, the better quality and
  more correct the visualization will be, but the trade-off is that it
  will also influence the rendering performance.

  The default value is SoVolumeRender::ALL.

  (The rendering performance can be \e severly improved by changing
  this field's value to either SoVolumeRender::MANUAL or
  SoVolumeRender::AUTOMATIC, and then tuning the
  SoVolumeRender::numSlices field.)
*/

/*!
  \var SoSFInt32 SoVolumeRender::numSlices

  Decides how many slices to render if
  SoVolumeRender::numSlicesControl is set to either
  SoVolumeRender::MANUAL or SoVolumeRender::AUTOMATIC.

  For \c MANUAL, it sets an absolute number. For \c AUTOMATIC, a
  calculation will be done based on the value of this field and the
  current SoComplexity::value setting in the scene graph traversal
  state.

  Note that the default value of the field is 0.
*/

// *************************************************************************

/*!
  \enum SoVolumeRender::Composition

  Enumeration of available types of composition for partly translucent
  volumes.
*/
/*!
  \var SoVolumeRender::Composition SoVolumeRender::ALPHA_BLENDING

  Composes volume by rendering voxels with higher opacity such that
  they progressively obscures voxels behind them.

  This is an approximation of the visual appearance of the penetration
  and reflection of light through a transparent material.

  The blending function for this is known as the "over" operator.
*/
/*!
  \var SoVolumeRender::Composition SoVolumeRender::MAX_INTENSITY

  For each on-screen projected pixel, the voxel with the highest alpha
  intensity along that projection ray will be rendered.

  This is for instance useful in medical imaging as a contrast
  enhancing operator for visualizing blood-flows.

  Note that the availability of this composition mode for rendering
  with 2D- and 3D-textureslices will be dependent on features of the
  underlying rendering library.

  (Specifically, the OpenGL driver must support \c glBlendEquation(),
  which is part of the optional "imaging" API-subset of OpenGL version
  1.2 and later.)
*/
/*!
  \var SoVolumeRender::Composition SoVolumeRender::SUM_INTENSITY

  For each on-screen projected pixel, the intensity of all voxels
  along that projection ray will be summed up before rendering.

  This gives an appearance similar to medical X-ray images.  The
  blending function is known as the "attenuate" operator.

  Note that the availability of this composition mode for rendering
  with 2D- and 3D-textureslices will be dependent on features of the
  underlying rendering library.

  (Specifically, the OpenGL driver must support \c glBlendEquation(),
  which is part of the optional "imaging" API-subset of OpenGL version
  1.2 and later.)
*/

/*!
  \var SoSFEnum SoVolumeRender::composition

  How to compose the projected volume rendering.  See
  SoVolumeRender::Composition.

  Default value is SoVolumeRender::ALPHA_BLENDING.
*/


// *************************************************************************

SoVolumeRender::SoVolumeRender(void)
{
  SO_NODE_CONSTRUCTOR(SoVolumeRender);

  PRIVATE(this) = new SoVolumeRenderP(this);

  SO_NODE_DEFINE_ENUM_VALUE(Interpolation, NEAREST);
  SO_NODE_DEFINE_ENUM_VALUE(Interpolation, LINEAR);
  SO_NODE_SET_SF_ENUM_TYPE(interpolation, Interpolation);

  SO_NODE_DEFINE_ENUM_VALUE(Composition, MAX_INTENSITY);
  SO_NODE_DEFINE_ENUM_VALUE(Composition, SUM_INTENSITY);
  SO_NODE_DEFINE_ENUM_VALUE(Composition, ALPHA_BLENDING);
  SO_NODE_SET_SF_ENUM_TYPE(composition, Composition);

  SO_NODE_DEFINE_ENUM_VALUE(NumSlicesControl, ALL);
  SO_NODE_DEFINE_ENUM_VALUE(NumSlicesControl, MANUAL);
  SO_NODE_DEFINE_ENUM_VALUE(NumSlicesControl, AUTOMATIC);
  SO_NODE_SET_SF_ENUM_TYPE(numSlicesControl, NumSlicesControl);

  SO_NODE_ADD_FIELD(interpolation, (SoVolumeRender::LINEAR));
  SO_NODE_ADD_FIELD(composition, (SoVolumeRender::ALPHA_BLENDING));
  SO_NODE_ADD_FIELD(lighting, (FALSE));
  SO_NODE_ADD_FIELD(lightDirection, (SbVec3f(-1, -1, -1)));
  SO_NODE_ADD_FIELD(lightIntensity, (1.0));
  SO_NODE_ADD_FIELD(numSlicesControl, (SoVolumeRender::ALL));
  SO_NODE_ADD_FIELD(numSlices, (0));
  SO_NODE_ADD_FIELD(viewAlignedSlices, (FALSE));

}

SoVolumeRender::~SoVolumeRender()
{
  delete PRIVATE(this);
}

// Doc from parent class.
void
SoVolumeRender::initClass(void)
{
  SO_NODE_INIT_CLASS(SoVolumeRender, SoShape, "SoShape");

  SO_ENABLE(SoGLRenderAction, SoTransferFunctionElement);
  SO_ENABLE(SoGLRenderAction, SoModelMatrixElement);
  SO_ENABLE(SoGLRenderAction, SoLazyElement);

  SO_ENABLE(SoRayPickAction, SoTransferFunctionElement);
  SO_ENABLE(SoRayPickAction, SoModelMatrixElement);

  SO_ENABLE(SoGLRenderAction, CvrGLInterpolationElement);
}

// doc in super
void
SoVolumeRender::GLRender(SoGLRenderAction * action)
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
  //
  // FIXME: we really shouldn't do it like this, we should rather let
  // SoShape::shouldGLRender() handle it automatically -- there could
  // be bad side effects from doing it like this (what will for
  // instance happen if there is transparent materials or textures
  // from graph traversal? Do we get pushed onto the delayed path list
  // of the action twice?).
  //
  // To have SoShape::shouldGLRender() handle it automatically, we
  // need to "fake" transparency onto the state stack, e.g. by pushing
  // a material element. Investigate the SoShape::shouldGLRender()
  // code to find out exactly how. 20040707 mortene.
  if (!action->isRenderingDelayedPaths()) {
    action->addDelayedPath(action->getCurPath()->copy());
    return;
  }

  if (CvrUtil::debugRayPicks()) { PRIVATE(this)->rayPickDebug(action); }
  SoState * state = action->getState();

  // Fetching the current volumedata
  const CvrVoxelBlockElement * vbelement = CvrVoxelBlockElement::getInstance(state);
  const SbVec3s & voxcubedims = vbelement->getVoxelCubeDimensions();

  if (vbelement == NULL) {
    static SbBool first = TRUE;
    if (first) {
      SoDebugError::postWarning("SoVolumeRender::GLRender",
                                "No SoVolumeData in scene graph before "
                                "SoVolumeRender node.");
      first = FALSE;
    }
    return;
  }


  // Shall we draw the volume as lines/wireframe/points?
  SoDrawStyleElement::Style drawstyle = SoDrawStyleElement::get(state);
  if (drawstyle == SoDrawStyleElement::LINES ||
      drawstyle == SoDrawStyleElement::POINTS) { 

    // Is the line/point style volume cube created yet?
    if (!PRIVATE(this)->linestylevolumecube) {
      PRIVATE(this)->linestylevolumecube = new SoCube;
      PRIVATE(this)->linestylevolumecube->ref();     
    }
   
    const SbBox3f volsize = vbelement->getUnitDimensionsBox();
    float dx, dy, dz; // The size might have changed since last time.
    volsize.getSize(dx, dy, dz); 
    PRIVATE(this)->linestylevolumecube->width.setValue(dx);
    PRIVATE(this)->linestylevolumecube->height.setValue(dy);
    PRIVATE(this)->linestylevolumecube->depth.setValue(dz);    
    PRIVATE(this)->linestylevolumecube->GLRender(action);
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
    // FIXME: should instead just use a default transferfunction.
    // Perhaps SoVolumeData (?) could place one on the state stack?
    // 20040220 mortene.
    static SbBool first = TRUE;
    if (first) {
      SoDebugError::postWarning("SoVolumeRender::GLRender",
                                "No SoTransferFunction in scene graph before "
                                "SoVolumeRender node -- rendering aborted.");
      first = FALSE;
    }
    return;
  }


#if CVR_DEBUG && 0 // debug
  SoDebugError::postInfo("SoVolumeRender::GLRender", "voxcubedims==[%d, %d, %d]",
                         voxcubedims[0], voxcubedims[1], voxcubedims[2]);
#endif // debug
  assert((voxcubedims[0] >= 0) && (voxcubedims[1] >= 0) && (voxcubedims[2] >= 0));
  if (voxcubedims[0] == 0) {
    assert((voxcubedims[1] == 0) && (voxcubedims[2] == 0));
    static SbBool first = TRUE;
    if (first) {
      SoDebugError::postWarning("SoVolumeRender::GLRender",
                                "No valid volume seems to be available from "
                                "the previous SoVolumeData.");
      first = FALSE;
    }
    return;
  }

  state->push();
  // *** Below this point, don't invoke "return", but do "goto done;",
  // *** so state->pop() is done before returning.

  SbMatrix volumetransform;
  CvrUtil::getTransformFromVolumeBoxDimensions(vbelement, volumetransform);
  SoModelMatrixElement::mult(state, this, volumetransform);

  int rendermethod;

  static int renderwithglpoints = -1;
  if (renderwithglpoints == -1) {
    const char * env = coin_getenv("CVR_FORCE_GLPOINTRENDERING");
    renderwithglpoints = env && (atoi(env) > 0);
  }

  if (renderwithglpoints) {
    PointRendering::render(action);
    goto done;
  }

  rendermethod = SoVolumeRenderP::TEXTURE2D; // we consider this the default

  if (!CvrUtil::force2DTextureRendering()) {
    const int storagehint = CvrStorageHintElement::get(state);
    if (storagehint == SoVolumeData::TEX3D ||
        storagehint == SoVolumeData::AUTO ||
        // FIXME: should also warn on attempts to use the VOLUMEPRO
        // setting. 20040712 mortene.
        storagehint == SoVolumeData::VOLUMEPRO) {
      const cc_glglue * glue = cc_glglue_instance(action->getCacheContext());
      if (!cc_glglue_has_3d_textures(glue)) {
        static SbBool first = TRUE;
        if (first && CvrUtil::doDebugging()) {
          first = FALSE;
          SoDebugError::postInfo("SoVolumeRender::GLRender",
                                 "The OpenGL driver does not support "
                                 "3D texturing, will fall back on 2D textures.");
        }
      }
      else if (PRIVATE(this)->use3DTexturing(glue)) {
        rendermethod = SoVolumeRenderP::TEXTURE3D;
      }
    }
  }

  GLenum interp;
  switch (this->interpolation.getValue()) {
  case NEAREST: interp = GL_NEAREST; break;
  case LINEAR: interp = GL_LINEAR; break;
  default: assert(FALSE && "invalid value in interpolation field"); break;
  }

  CvrGLInterpolationElement::set(state, interp);

  // viewport-aligned 3D textures
  if (rendermethod == SoVolumeRenderP::TEXTURE3D) {

    const int numslices = PRIVATE(this)->calculateNrOf3DSlices(action, voxcubedims);
    if (numslices == 0) goto done;

    if (!PRIVATE(this)->cubehandler) {
      PRIVATE(this)->cubehandler = new CvrCubeHandler();
    }

    CvrCubeHandler::Composition composit;
    switch (this->composition.getValue()) {
    case ALPHA_BLENDING: composit = CvrCubeHandler::ALPHA_BLENDING; break;
    case MAX_INTENSITY: composit = CvrCubeHandler::MAX_INTENSITY; break;
    case SUM_INTENSITY: composit = CvrCubeHandler::SUM_INTENSITY; break;
    default: assert(FALSE && "invalid value in composition field"); break;
    }

    // FIXME: wouldn't it be better to push composition info onto the
    // state stack instead? 20040715 mortene.
    PRIVATE(this)->cubehandler->render(action, numslices, composit,
                                       PRIVATE(this)->abortfunc,
                                       PRIVATE(this)->abortfuncdata);

  }
  // axis-aligned 2D textures
  else if (rendermethod == SoVolumeRenderP::TEXTURE2D) {
    // let model matrix contain *all* scaling, so the render code can
    // simply work with a unit cube:
    SoModelMatrixElement::scaleBy(state, this, SbVec3f(voxcubedims[0], voxcubedims[1], voxcubedims[2]));

    const int numslices = PRIVATE(this)->calculateNrOf2DSlices(action, voxcubedims);
    if (numslices == 0) goto done;

    if (!PRIVATE(this)->pagehandler) {
      PRIVATE(this)->pagehandler = new CvrPageHandler(action);
    }

    CvrPageHandler::Composition composit;
    switch (this->composition.getValue()) {
    case ALPHA_BLENDING: composit = CvrPageHandler::ALPHA_BLENDING; break;
    case MAX_INTENSITY: composit = CvrPageHandler::MAX_INTENSITY; break;
    case SUM_INTENSITY: composit = CvrPageHandler::SUM_INTENSITY; break;
    default: assert(FALSE && "invalid value in composition field"); break;
    }

    // FIXME: wouldn't it be better to push composition info onto the
    // state stack instead? 20040715 mortene.
    PRIVATE(this)->pagehandler->render(action, numslices, composit,
                                       PRIVATE(this)->abortfunc,
                                       PRIVATE(this)->abortfuncdata);
  }
  else {
    assert(FALSE && "Rendering method not implemented/supported.");
  }


done:

  // Enable this to do many 3d-to-2d-texture-performance tests.
  if (SoVolumeRenderP::debug3DTextureTiming()) {
    const cc_glglue * glw = cc_glglue_instance(action->getCacheContext());
    (void)SoVolumeRenderP::performanceTest(glw);
  }

  state->pop();
}

// doc in super
void
SoVolumeRender::generatePrimitives(SoAction * action)
{
  // FIXME: implement me? 20021120 mortene.

#if CVR_DEBUG && 1 // debug
  static SbBool warn = TRUE;
  if (warn) {
    SoDebugError::postInfo("SoVolumeRender::generatePrimitives",
                           "not yet implemented");
    warn = FALSE;
  }
#endif // debug
}

/*!
  Picking of a volume doesn't work in quite the same manner as picking
  polygon geometry: the SoPickedPoint set up in the SoRayPickAction
  class will only contain the entry point of the ray into the
  volume.

  For further picking information, grab the detail object and cast it
  to an SoVolumeRenderDetail (after first checking that it is of this
  type, of course).
 */
void
SoVolumeRender::rayPick(SoRayPickAction * action)
{
  if (!this->shouldRayPick(action)) return;

  SbVec3f intersects[2];  
  SoState * state = action->getState();  
  this->computeObjectSpaceRay(action);

  if (!cvr_volumeraypickintersection(action, intersects))
    return;
      
  if (CvrUtil::debugRayPicks()) {
    PRIVATE(this)->raypicklines.append(intersects[0]);
    PRIVATE(this)->raypicklines.append(intersects[1]);
    this->touch(); // smash caches and re-render with line(s)
  }
 
  SoVolumeRenderDetail * detail = new SoVolumeRenderDetail;
  detail->setDetails(intersects[0], intersects[1], state, this);

}

// doc in super
void
SoVolumeRender::computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center)
{
  SoState * state = action->getState();

  const CvrVoxelBlockElement * vbelem = CvrVoxelBlockElement::getInstance(state);
  if (vbelem == NULL) { return; }

  const SbBox3f & vdbox = vbelem->getUnitDimensionsBox();
  if (vdbox.isEmpty()) { return; }

  box.extendBy(vdbox);
  center = vdbox.getCenter();
}

/*!
  \enum SoVolumeRender::AbortCode

  The set of valid values that should be returned from a function set
  up in SoVolumeRender::setAbortCallback().
*/
/*!
  \var SoVolumeRender::AbortCode SoVolumeRender::CONTINUE
  Continue rendering in the usual manner.
*/
/*!
  \var SoVolumeRender::AbortCode SoVolumeRender::ABORT
  Don't render any more textured slices of the volume.
*/
/*!
  \var SoVolumeRender::AbortCode SoVolumeRender::SKIP
  Skip the next textured slice, and resume rendering on the next after
  that. (The abort callback function will still be called again.)
*/
/*!
  \typedef AbortCode SoVolumeRender::SoVolumeRenderAbortCB(int totalslices, int thisslice, void * userdata)

  The function signature for callback function pointers to be passed
  in to SoVolumeRender::setAbortCallback().

  \a totalslices is the total number of textured slices that is
  expected to be rendered, unless the callback choose to abort or skip
  any of them.

  \a thisslice is the index number of the next slice to render. Note
  that they are rendered back-to-front, and that they are numbered
  from 1 to \a totalslices.

  \a userdata is the second argument given to
  SoVolumeRender::setAbortCallback() when the callback was set up.
*/

/*!
  Lets the application programmer supply a callback function, by which
  it will be possible to either prematurely abort the rendering of a
  set of slices, or to skip certain slices.

  Both of these measures are of course optimizations of rendering
  performance controlled from client code.
*/
void
SoVolumeRender::setAbortCallback(SoVolumeRenderAbortCB * func, void * userdata)
{
  PRIVATE(this)->abortfunc = func;
  PRIVATE(this)->abortfuncdata = userdata;
}

// Will render the intersection lines for all ray picks attempted so
// far. For debugging purposes only.
void
SoVolumeRenderP::rayPickDebug(SoGLRenderAction * action)
{
  SoState * state = action->getState();
  state->push();

  static SoColorPacker * cp = new SoColorPacker;
  static SbColor linecol(1, 1, 0);
  SoLazyElement::setDiffuse(state, PUBLIC(this), 1, &linecol, cp);

  // disable lighting
  SoLazyElement::setLightModel(state, SoLazyElement::BASE_COLOR);
  // disable texture mapping
  SoGLTextureEnabledElement::set(state, PUBLIC(this), FALSE);
  SoGLTexture3EnabledElement::set(state, PUBLIC(this), FALSE);

  SoMaterialBundle mb(action);
  mb.sendFirst(); // set current color

  for (int i=0; i < this->raypicklines.getLength(); i += 2) {
    const SbVec3f & v0 = this->raypicklines[i];
    const SbVec3f & v1 = this->raypicklines[i + 1];
    glBegin(GL_LINES);
    glVertex3f(v0[0], v0[1], v0[2]);
    glVertex3f(v1[0], v1[1], v1[2]);
    glEnd();
  }

  state->pop();
}

// *************************************************************************

void
SoVolumeRenderP::setupPerformanceTestTextures(GLuint * texture3did,
                                              GLuint * texture2dids,
                                              const cc_glglue * glglue)
{
  const unsigned int TEXTUREDIMENSION = 16;
  const unsigned int TEXTUREBUFSIZE =
    TEXTUREDIMENSION * TEXTUREDIMENSION * TEXTUREDIMENSION * 4;

  uint8_t * volumedataset = new uint8_t[TEXTUREBUFSIZE];
  for (unsigned int i = 0; i < TEXTUREBUFSIZE; i++) {
    volumedataset[i] = (uint8_t)(rand() % 256);
  }

  // Setup 3D texture
  glGenTextures(1, texture3did);
  assert(glGetError() == GL_NO_ERROR);

  glEnable(GL_TEXTURE_3D);
  glBindTexture(GL_TEXTURE_3D, texture3did[0]);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
  cc_glglue_glTexImage3D(glglue,
                         GL_TEXTURE_3D, 0, GL_RGBA,
                         TEXTUREDIMENSION, TEXTUREDIMENSION, TEXTUREDIMENSION,
                         0, GL_RGBA, GL_UNSIGNED_BYTE, volumedataset);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  // Setup 2D textures
  glGenTextures(2, texture2dids);
  assert(glGetError() == GL_NO_ERROR);

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, texture2dids[0]);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEXTUREDIMENSION, TEXTUREDIMENSION,
               0, GL_RGBA, GL_UNSIGNED_BYTE, volumedataset);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glBindTexture(GL_TEXTURE_2D, texture2dids[1]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEXTUREDIMENSION, TEXTUREDIMENSION,
               0, GL_RGBA, GL_UNSIGNED_BYTE, volumedataset);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  delete[] volumedataset;
}

static inline void
render_textured_triangle(const SbVec3f * v)
{
  glBegin(GL_TRIANGLES);
  glTexCoord3f(0.0f, 0.0f, 0.0f);
  glVertex3fv(v[0].getValue());
  glTexCoord3f(1.0f, 0.0f, 0.0f);
  glVertex3fv(v[1].getValue());
  glTexCoord3f(0.0f, 1.0f, 1.0f);
  glVertex3fv(v[2].getValue());
  glEnd();
}

void
SoVolumeRenderP::renderPerformanceTestScene(SbList<double> & timelist3d,
                                            SbList<double> & timelist2d,
                                            GLuint * texture3did,
                                            GLuint * texture2dids)
{
  // All the rendered triangles should have the same size to prevent
  // un-even measurements, which could happen if we set up random
  // coordinates in "fluke" cases where a string of polygons all have
  // excessively small screen space. This would bias the measurements
  // to give too small values for the 3d-to-2d ratio on systems
  // without 3d-texturing in hardware.
  const SbVec3f v[3] = {
    SbVec3f(-0.2f, -0.2f, 0), SbVec3f(0.2f, -0.2f, 0), SbVec3f(0.2f, 0.2f, 0)
  };

  // 3D texture
  SbTime t = SbTime::getTimeOfDay();

  glDisable(GL_TEXTURE_2D);
  glEnable(GL_TEXTURE_3D);
  unsigned int i;
  for (i = 0; i < VOLUMERENDER_PERFORMANCETEST_TRIANGLES; ++i) {
    glBindTexture(GL_TEXTURE_3D, texture3did[0]);
    render_textured_triangle(v);
  }

  glFinish();

  timelist3d.append((SbTime::getTimeOfDay() - t).getValue());

  // 2D textures
  t = SbTime::getTimeOfDay();

  glDisable(GL_TEXTURE_3D);
  glEnable(GL_TEXTURE_2D);
  for (i = 0; i < VOLUMERENDER_PERFORMANCETEST_TRIANGLES; ++i) {
    glBindTexture(GL_TEXTURE_2D, texture2dids[i & 1]); // Flip between two textures
    render_textured_triangle(v);
  }

  glFinish();

  timelist2d.append((SbTime::getTimeOfDay() - t).getValue());
}

double
SoVolumeRenderP::getAveragePerformanceTime(SbList<double> & l)
{
  assert(l.getLength() > 0);
  if (l.getLength() == 1) { return l[0]; }

  unsigned int i;

  // Remove the highest and lowest value in the array, in case
  // e.g. the initial uploading of textures took especially much time:
  if (l.getLength() >= 4) {
    unsigned int idhighest = UINT_MAX, idlowest = UINT_MAX;
    double highest = -DBL_MAX, lowest = DBL_MAX;
    for (i = 0; i < (unsigned int)l.getLength(); ++i) {
      if (l[i] < lowest) {
        idlowest = i;
        lowest = l[i];
      }
    }
    assert(idlowest != UINT_MAX);
    l.removeFast(idlowest);

    for (i = 0; i < (unsigned int)l.getLength(); ++i) {
      if (l[i] > highest) {
        idhighest = i;
        highest = l[i];
      }
    }
    assert(idhighest != UINT_MAX);
    l.removeFast(idhighest);

    if (CvrUtil::doDebugging()) {
      SoDebugError::postInfo("SoVolumeRenderP::getAveragePerformanceTime",
                             "worst, best, ratio: %f, %f, %f",
                             highest, lowest, highest / lowest);
    }
  }

  double sum = 0;
  for (i = 0; i < (unsigned int)l.getLength(); ++i) { sum += l[i]; }
  return (sum / l.getLength());
}

double
SoVolumeRenderP::performanceTest(const cc_glglue * glue)
{
  const SbTime t = SbTime::getTimeOfDay();
  if (CvrUtil::doDebugging()) {
    SoDebugError::postInfo("SoVolumeRenderP::performanceTest",
                           "start at %f", t.getValue());
  }

  glPushAttrib(GL_ALL_ATTRIB_BITS);

  GLuint texture3did[1];
  GLuint texture2dids[2];
  SoVolumeRenderP::setupPerformanceTestTextures(texture3did, texture2dids, glue);

  glPixelTransferf(GL_RED_SCALE, 1.0f);  // Setting to initial values
  glPixelTransferf(GL_GREEN_SCALE, 1.0f);
  glPixelTransferf(GL_BLUE_SCALE, 1.0f);
  glPixelTransferf(GL_ALPHA_SCALE, 1.0f);
  glPixelTransferf(GL_RED_BIAS, 0.0f);
  glPixelTransferf(GL_GREEN_BIAS, 0.0f);
  glPixelTransferf(GL_BLUE_BIAS, 0.0f);
  glPixelTransferf(GL_ALPHA_BIAS, 0.0f);  
  glPixelZoom(1.0f, 1.0f);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
  
  // Save the framebuffer for later
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  const unsigned int size = viewport[2] * viewport[3] * 4;
  GLubyte * framebuf = new GLubyte[size];
  glReadPixels(0, 0, viewport[2], viewport[3], GL_RGBA, GL_UNSIGNED_BYTE, framebuf);

  glDepthMask(GL_FALSE); // Dont write to the depthbuffer. It wont be restored.
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glDisable(GL_CULL_FACE);
  glDisable(GL_BLEND);

  glViewport(0, 0, viewport[2], viewport[3]);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();


  // The following is a substitute for gluPerspective(...)
  const float fovy = 45.0f;
  const float zNear = 0.1f;
  const float zFar = 10.0f;
  const float aspect = (float) viewport[2] / (float) viewport[3];
  const double radians = fovy / 2 * M_PI / 180;
  const double deltaZ = zFar - zNear;
  const double sine = sin(radians);
  const double cotangent = cos(radians) / sine;
  GLdouble m[4][4];
  m[0][1] = 0; m[0][2] = 0; m[0][3] = 0;
  m[1][0] = 0; m[1][2] = 0; m[1][3] = 0;
  m[2][0] = 0; m[2][1] = 0;
  m[3][0] = 0; m[3][1] = 0;
  m[0][0] = cotangent / aspect;
  m[1][1] = cotangent;
  m[2][2] = -(zFar + zNear) / deltaZ;
  m[2][3] = -1;
  m[3][2] = -2 * zNear * zFar / deltaZ;
  m[3][3] = 0;
  glMultMatrixd(&m[0][0]); // Finished

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glTranslatef(0, 0, -0.5f); // Move camera a bit, so polygons can be
                             // rendered at z=0.
  
  if (SoVolumeRenderP::debug3DTextureTiming()) {
    glClearColor(0, 0, 1, 0);
    glClear(GL_COLOR_BUFFER_BIT);
  }
  
  // Make sure we don't drag along the full pipeline into the first
  // test run, by forcing completion of all GL commands currently
  // being processed.
  glFinish();

  
  SbList<double> timelist2d, timelist3d;
  SbTime starttime = SbTime::getTimeOfDay();
 
  for (unsigned int i = 0; i < VOLUMERENDER_PERFORMANCETEST_TIMES; ++i) {
    
    SoVolumeRenderP::renderPerformanceTestScene(timelist3d, timelist2d,
                                                texture3did, texture2dids);
            
    const SbTime now = SbTime::getTimeOfDay();
#if 0 // debug
    if (CvrUtil::doDebugging()) {
      SoDebugError::postInfo("SoVolumeRenderP::performanceTest",
                             "run %u done at %f", i, now.getValue());
    }
#endif

    // Don't run the test for more than half a second.
    if (((now - starttime).getValue()) > .5f) { break; }
  }
  
  if (CvrUtil::doDebugging()) {
    SoDebugError::postInfo("SoVolumeRenderP::performanceTest",
                           "managed %d runs", timelist2d.getLength());
  }
  
  const double average3dtime = SoVolumeRenderP::getAveragePerformanceTime(timelist3d);
  const double average2dtime = SoVolumeRenderP::getAveragePerformanceTime(timelist2d);
 
  glDeleteTextures(1, texture3did);
  glDeleteTextures(2, texture2dids);

  // Write back framebuffer
  if (!SoVolumeRenderP::debug3DTextureTiming()) {
    
    glViewport(0, 0, viewport[2], viewport[3]);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, viewport[2], 0, viewport[3], -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
         
    glRasterPos2i(0, 0);
    glDisable(GL_DEPTH_TEST);
    glDrawPixels(viewport[2], viewport[3], GL_RGBA, GL_UNSIGNED_BYTE, framebuf);

  }
  delete[] framebuf;

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glPopAttrib(); 

  if (CvrUtil::doDebugging()) {
    SoDebugError::postInfo("SoVolumeRenderP::performanceTest",
                           "texture performance test results: "
                           "average3dtime==%f, average2dtime==%f --> "
                           "rating(3D/2D)==%f"
                           "   (spent %f seconds in testing)",
                           average3dtime, average2dtime,
                           (average3dtime / average2dtime),
                           (SbTime::getTimeOfDay() - t).getValue());
  }



  return average3dtime / average2dtime;
}


SbBool
SoVolumeRenderP::use3DTexturing(const cc_glglue * glglue) const
{
  // This check should only be done once.
  //
  // FIXME: not correct -- it should be done once for each new GL
  // context that we need to render in. 20040716 mortene.
  static int do3dtextures = -1;

  if (do3dtextures == 1) return TRUE;
  else if (do3dtextures == 0) return FALSE;

  // Shall we force 3D texturing?
  const char * envstr = coin_getenv("CVR_FORCE_3D_TEXTURES");
  if (envstr && (atoi(envstr) > 0)) {
    do3dtextures = 1;
    return TRUE;
  }

  // Setting this environment variable forces the code below to skip
  // the whitelist / blacklist checking, and instead always run the
  // performance test.
  envstr = coin_getenv("CVR_NO_3D_ACCELERATION_CHECKLISTS");
  const SbBool skiptests = (envstr && (atoi(envstr) > 0)) ? TRUE : FALSE;

  static const GLubyte * rendererstring = glGetString(GL_RENDERER);
  unsigned int i=0;
  while (!skiptests && texture3d_in_hardware[i]) {
    const char * loc = strstr((const char *)rendererstring,
                              texture3d_in_hardware[i++]);
    if (loc != NULL) {
      do3dtextures = 1;
      if (CvrUtil::doDebugging()) {
        SoDebugError::postInfo("SoVolumeRenderP::use3DTexturing",
                               "Your OpenGL driver and graphics card "
                               "combination ('%s') was found in a whitelist "
                               "of drivers/cards able to accelerate "
                               "3D textures in hardware. "
                               "(If you wish to force 2D texturing instead, "
                               "set the envvar CVR_FORCE_2D_TEXTURES=1).",
                               rendererstring);
      }
      return TRUE;
    }
  }

  i=0;
  while (!skiptests && texture3d_in_software[i]) {
    const char * loc = strstr((const char *)rendererstring,
                              texture3d_in_software[i++]);
    if (CvrUtil::doDebugging() && loc) {
      SoDebugError::postInfo("SoVolumeRenderP::use3DTexturing",
                             "Your OpenGL driver has 3D texture abilities, "
                             "but your graphics card ('%s') was found in a "
                             "blacklist of cards not able to accelerate "
                             "3D textures in hardware. Falling back on 2D "
                             "textures instead. "
                             "(If you wish to force 3D texturing, set the "
                             "envvar CVR_FORCE_3D_TEXTURES=1).",
                             rendererstring);
      do3dtextures = 0;
      return FALSE;
    }
  }


  // FIXME: The performance test should be properly tested on many
  // different GFX cards to see if the rating threshold is high enough
  // (20040503 handegar)
  const double rating = SoVolumeRenderP::performanceTest(glglue); // => (3D rendertime) / (2D rendertime)
  if (rating < 10.0f) { // 2D should at least be this many times
                        // faster before 3D texturing is dropped.
    do3dtextures = 1;
    return TRUE;
  }

  if (CvrUtil::doDebugging()) {
    SoDebugError::postInfo("SoVolumeRenderP::use3DTexturing",
                           "Your GFX card did not score high enough in the "
                           "performance test for 3D textures compared to "
                           "2D textures. 2D texturing will be used. "
                           "(If you wish to force 3D texturing, set the "
                           "envvar CVR_FORCE_3D_TEXTURES=1)");
  }

  do3dtextures = 0;
  return FALSE;
}

unsigned int
SoVolumeRenderP::calculateNrOf2DSlices(SoGLRenderAction * action,
                                       const SbVec3s & dimensions)
{
  int numslices = 0;
  const int control = PUBLIC(this)->numSlicesControl.getValue();
  const unsigned int AXISIDX = this->pagehandler->getCurrentAxis(action);

  if ((control == SoVolumeRender::ALL) ||
      (PUBLIC(this)->numSlices.getValue() <= 0)) {
    numslices = dimensions[AXISIDX];
  }
  else if (control == SoVolumeRender::MANUAL) {
    numslices = PUBLIC(this)->numSlices.getValue();
  }
  else if (control == SoVolumeRender::AUTOMATIC) {
    const float complexity = PUBLIC(this)->getComplexityValue(action);
    numslices = int(complexity * 2.0f * PUBLIC(this)->numSlices.getValue());
    assert(numslices >= 0);
  }
  else {
    assert(FALSE && "invalid numSlicesControl value");
  }

#if CVR_DEBUG && 0 // debug
  SoDebugError::postInfo("SoVolumeRenderP::calculateNrOf2DSlices",
                         "numslices == %d", numslices);
#endif // debug

  return numslices;
}

unsigned int
SoVolumeRenderP::calculateNrOf3DSlices(SoGLRenderAction * action,
                                       const SbVec3s & dimensions)
{
  int numslices = 0;
  const int control = PUBLIC(this)->numSlicesControl.getValue();
  const float complexity = PUBLIC(this)->getComplexityValue(action);

  if ((control == SoVolumeRender::ALL) ||
      (PUBLIC(this)->numSlices.getValue() <= 0)) {
    // 'Applying' the Nyquist theorem
    numslices = (unsigned int) sqrt(double(dimensions[0]*dimensions[0] +
                                           dimensions[1]*dimensions[1] +
                                           dimensions[2]*dimensions[2])) * 2;
    numslices = int(complexity * 2.0f * numslices);
  }
  else if (control == SoVolumeRender::MANUAL) {
    numslices = PUBLIC(this)->numSlices.getValue() + 1;
  }
  else if (control == SoVolumeRender::AUTOMATIC) {
    numslices = int(complexity * 2.0f * PUBLIC(this)->numSlices.getValue());
    assert(numslices >= 0);
  }
  else {
    assert(FALSE && "invalid numSlicesControl value");
  }

#if CVR_DEBUG && 0 // debug
  SoDebugError::postInfo("SoVolumeRenderP::calculateNrOf3DSlices",
                         "numslices == %d", numslices);
#endif // debug

  return numslices;
}

// *************************************************************************

#undef PRIVATE
#undef PUBLIC


