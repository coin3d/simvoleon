#include <VolumeViz/nodes/SoVolumeRender.h>

#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoRayPickAction.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/system/gl.h>
#include <Inventor/SbLine.h>
#include <Inventor/SbPlane.h>
#include <Inventor/elements/SoLazyElement.h>
#include <Inventor/elements/SoGLTextureEnabledElement.h>
#include <Inventor/elements/SoGLTexture3EnabledElement.h>
#include <Inventor/bundles/SoMaterialBundle.h>
#include <Inventor/C/tidbits.h>
#include <Inventor/SoPickedPoint.h>

#include <VolumeViz/elements/SoTransferFunctionElement.h>
#include <VolumeViz/elements/SoVolumeDataElement.h>
#include <VolumeViz/nodes/SoVolumeData.h>
#include <VolumeViz/render/2D/CvrPageHandler.h>
#include <VolumeViz/details/SoVolumeRenderDetail.h>
#include <VolumeViz/misc/CvrVoxelChunk.h>
#include <VolumeViz/misc/CvrCLUT.h>
#include <VolumeViz/misc/CvrUtil.h>

// *************************************************************************

SO_NODE_SOURCE(SoVolumeRender);

// *************************************************************************

class SoVolumeRenderP {
public:
  SoVolumeRenderP(SoVolumeRender * master)
  {
    this->master = master;
    this->pagehandler = NULL;

    this->abortfunc = NULL;
    this->abortfuncdata = NULL;
  }

  ~SoVolumeRenderP()
  {
    delete this->pagehandler;
  }

  unsigned int calculateNrOfSlices(SoGLRenderAction * action,
                                   const SbVec3s & dimensions);

  CvrPageHandler * pagehandler;

  SoVolumeRender::SoVolumeRenderAbortCB * abortfunc;
  void * abortfuncdata;

  // debug
  void rayPickDebug(SoGLRenderAction * action);
  SbList<SbVec3f> raypicklines;

private:
  SoVolumeRender * master;
};

#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

unsigned int
SoVolumeRenderP::calculateNrOfSlices(SoGLRenderAction * action,
                                     const SbVec3s & dimensions)
{
  int numslices = 0;
  const int control = PUBLIC(this)->numSlicesControl.getValue();
  const unsigned int AXISIDX = this->pagehandler->getCurrentAxis(action);

  if (control == SoVolumeRender::ALL) {
    numslices = dimensions[AXISIDX];
  }
  else if (control == SoVolumeRender::MANUAL) {
    numslices = PUBLIC(this)->numSlices.getValue();
  }
  else if (control == SoVolumeRender::AUTOMATIC) {
    float complexity = PUBLIC(this)->getComplexityValue(action);
    numslices = int(complexity * 2.0f * PUBLIC(this)->numSlices.getValue());
    assert(numslices >= 0);
  }
  else {
    assert(FALSE && "invalid numSlicesControl value");
  }

#if CVR_DEBUG && 0 // debug
  SoDebugError::postInfo("SoVolumeRenderP::calculateNrOfSlices",
                         "numslices == %d", numslices);
#endif // debug

  return numslices;
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

  SO_ENABLE(SoGLRenderAction, SoVolumeDataElement);
  SO_ENABLE(SoGLRenderAction, SoTransferFunctionElement);

  SO_ENABLE(SoRayPickAction, SoVolumeDataElement);
  SO_ENABLE(SoRayPickAction, SoTransferFunctionElement);
}

// doc in super
void
SoVolumeRender::GLRender(SoGLRenderAction * action)
{
  // FIXME: need to make sure we're not cached in a renderlist

  if (!this->shouldGLRender(action)) return;

  if (CvrUtil::debugRayPicks()) { PRIVATE(this)->rayPickDebug(action); }

  SoState * state = action->getState();

  // Fetching the current volumedata
  const SoVolumeDataElement * volumedataelement =
    SoVolumeDataElement::getInstance(state);
  assert(volumedataelement != NULL);

  SoVolumeData * volumedata = volumedataelement->getVolumeData();
  if (volumedata == NULL) {
    static SbBool first = TRUE;
    if (first) {
      SoDebugError::post("SoVolumeRender::GLRender",
                         "no SoVolumeData in scene graph before "
                         "SoVolumeRender node");
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
    static SbBool first = TRUE;
    if (first) {
      SoDebugError::post("SoVolumeRender::GLRender",
                         "no SoTransferFunction in scene graph before "
                         "SoVolumeRender node");
      first = FALSE;
    }
    return;
  }

  const SbVec3s voxcubedims = volumedataelement->getVoxelCubeDimensions();

#if CVR_DEBUG && 0 // debug
  SoDebugError::postInfo("SoVolumeRender::GLRender",
                         "voxcubedims==[%d, %d, %d]",
                         voxcubedims[0], voxcubedims[1], voxcubedims[2]);
#endif // debug

  // FIXME: check that rendering should be done through 2D texture
  // slices. 20021124 mortene.
  if (1) {
    if (!PRIVATE(this)->pagehandler) {
      PRIVATE(this)->pagehandler =
        new CvrPageHandler(voxcubedims, volumedata->getReader());
      // FIXME: needs to be invalidated / deallocated when
      // SoVolumeData get replaced by one with different voxcubedims
    }

    int numslices = PRIVATE(this)->calculateNrOfSlices(action, voxcubedims);
    if (numslices == 0) return;

    Cvr2DTexSubPage::Interpolation interp;
    switch (this->interpolation.getValue()) {
    case NEAREST: interp = Cvr2DTexSubPage::NEAREST; break;
    case LINEAR: interp = Cvr2DTexSubPage::LINEAR; break;
    default: assert(FALSE && "invalid value in interpolation field"); break;
    }

    CvrPageHandler::Composition composit;
    switch (this->composition.getValue()) {
    case ALPHA_BLENDING: composit = CvrPageHandler::ALPHA_BLENDING; break;
    case MAX_INTENSITY: composit = CvrPageHandler::MAX_INTENSITY; break;
    case SUM_INTENSITY: composit = CvrPageHandler::SUM_INTENSITY; break;
    default: assert(FALSE && "invalid value in composition field"); break;
    }

    PRIVATE(this)->pagehandler->render(action, numslices, interp, composit,
                                       PRIVATE(this)->abortfunc,
                                       PRIVATE(this)->abortfuncdata);
  }
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

  SoState * state = action->getState();

  const SoVolumeDataElement * volumedataelement = SoVolumeDataElement::getInstance(state);
  assert(volumedataelement != NULL);
  const SoVolumeData * volumedata = volumedataelement->getVolumeData();
  if (volumedata == NULL) { return; }


  this->computeObjectSpaceRay(action);
  const SbLine & ray = action->getLine();

  SbBox3f objbbox = volumedata->getVolumeSize();

  SbVec3f mincorner, maxcorner;
  objbbox.getBounds(mincorner, maxcorner);

  SbPlane sides[6] = {
    SbPlane(SbVec3f(0, 0, 1), mincorner[2]), // front face
    SbPlane(SbVec3f(0, 0, 1), maxcorner[2]), // back face
    SbPlane(SbVec3f(1, 0, 0), mincorner[0]), // left face
    SbPlane(SbVec3f(1, 0, 0), maxcorner[0]), // right face
    SbPlane(SbVec3f(0, 1, 0), mincorner[1]), // bottom face
    SbPlane(SbVec3f(0, 1, 0), maxcorner[1]) // top face
  };

  static int cmpindices[6][2] = { { 0, 1 }, { 1, 2 }, { 0, 2 } };

  unsigned int nrintersect = 0;
  SbVec3f intersects[2];
  for (unsigned int i=0; i < (sizeof(sides) / sizeof(sides[0])); i++) {
    SbVec3f intersectpt;
    if (sides[i].intersect(ray, intersectpt)) {
      const int axisidx0 = cmpindices[i / 2][0];
      const int axisidx1 = cmpindices[i / 2][1];

      if ((intersectpt[axisidx0] >= mincorner[axisidx0]) &&
          (intersectpt[axisidx0] <= maxcorner[axisidx0]) &&
          (intersectpt[axisidx1] >= mincorner[axisidx1]) &&
          (intersectpt[axisidx1] <= maxcorner[axisidx1])) {
        intersects[nrintersect++] = intersectpt;
        // Break if we happen to hit more than three sides (could
        // perhaps happen in borderline cases).
        if (nrintersect == 2) break;
      }
    }
  }


  // Borderline case, ignore pick attempt.
  if (nrintersect < 2) { return; }

  assert(nrintersect == 2);

  // Sort so first index is the one closest to ray start.
  if ((ray.getPosition() - intersects[0]).sqrLength() >
      (ray.getPosition() - intersects[1]).sqrLength()) {
    SbSwap(intersects[0], intersects[1]);
  }

  if (CvrUtil::debugRayPicks()) {
    PRIVATE(this)->raypicklines.append(intersects[0]);
    PRIVATE(this)->raypicklines.append(intersects[1]);
    this->touch(); // smash caches and re-render with line(s)
  }

  const SoTransferFunctionElement * transferfunctionelement =
    SoTransferFunctionElement::getInstance(state);
  assert(transferfunctionelement != NULL);
  SoTransferFunction * transferfunction =
    transferfunctionelement->getTransferFunction();
  assert(transferfunction != NULL);

  const SbVec3s voxcubedims = volumedataelement->getVoxelCubeDimensions();

  // Find objectspace-dimensions of a voxel.
  const SbVec3f size = maxcorner - mincorner;
  const SbVec3f voxelsize(voxcubedims[0] / size[0],
                          voxcubedims[1] / size[1],
                          voxcubedims[2] / size[2]);

  // Calculate maximum number of voxels that could possibly be touched
  // by the ray.
  const SbVec3f rayvec = (intersects[1] - intersects[0]);
  const float minvoxdim = SbMin(voxelsize[0], SbMin(voxelsize[1], voxelsize[2]));
  const unsigned int maxvoxinray = (unsigned int)(rayvec.length() / minvoxdim + 1);

  const SbVec3f stepvec = rayvec / maxvoxinray;
  SbVec3s ijk, lastijk(-1, -1, -1);

  const SbBox3s voxelbounds(SbVec3s(0, 0, 0), voxcubedims - SbVec3s(1, 1, 1));
  const SoVolumeData::DataType voxeltype = volumedataelement->getVoxelDataType();

  SoPickedPoint * pp = NULL;
  SoVolumeRenderDetail * detail = NULL;
  CvrCLUT * clut = NULL;
  SbVec3f objectcoord = intersects[0];

  while (TRUE) {
    // FIXME: we're not hitting the voxels in an exact manner with the
    // intersection testing (it seems we're slightly off in the
    // x-direction, at least), as can be seen from the
    // SimVoleon/testcode/raypick example (either that or it could be
    // the actual 2D texture-slice rendering that is wrong). 20030220 mortene.
    //
    // UPDATE: this might have been fixed now, at least I found and
    // fixed an offset bug in the objectCoordsToIJK() method
    // today. 20030320 mortene.

    ijk = volumedataelement->objectCoordsToIJK(objectcoord);
    if (!voxelbounds.intersect(ijk)) break;

    if (!action->isBetweenPlanes(objectcoord)) {
      objectcoord += stepvec;
      continue;
    }

    if (ijk != lastijk) { // touched new voxel
      lastijk = ijk;
      if (CvrUtil::debugRayPicks()) {
        SoDebugError::postInfo("SoVolumeRender::rayPick",
                               "ijk=<%d, %d, %d>", ijk[0], ijk[1], ijk[2]);
      }
      if (pp == NULL) {
        pp = action->addIntersection(objectcoord);
        // if NULL, something else is obstructing the view to the
        // volume, the app programmer only want the nearest, and we
        // don't need to continue our intersection tests
        if (pp == NULL) return;
        // FIXME: should fill in the normal vector of the pickedpoint:
//         pp->setObjectNormal(<voxcube-side-normal>);
        // 20030320 mortene.

        detail = new SoVolumeRenderDetail;
        pp->setDetail(detail, this);

        clut = CvrVoxelChunk::getCLUT(transferfunctionelement);
        clut->ref();
      }

      const uint32_t voxelvalue = volumedata->getVoxelValue(ijk);
      uint8_t rgba[4];
      if (voxeltype == SoVolumeData::RGBA) { *((uint32_t *)rgba) = voxelvalue; }
      else { clut->lookupRGBA(voxelvalue, rgba); }

      detail->addVoxelIntersection(objectcoord, ijk, voxelvalue, rgba);
    }
    else if (CvrUtil::debugRayPicks()) {
      SoDebugError::postInfo("SoVolumeRender::rayPick", "duplicate");
    }

    objectcoord += stepvec;
  }

  if (clut) clut->unref();
}

// doc in super
void
SoVolumeRender::computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center)
{
  SoState * state = action->getState();

  const SoVolumeDataElement * volumedataelement =
    SoVolumeDataElement::getInstance(state);

  if (volumedataelement == NULL) return;

  SoVolumeData * volumedata = volumedataelement->getVolumeData();

  SbBox3f vdbox = volumedata->getVolumeSize();
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
