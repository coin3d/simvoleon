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

  SbVec3s objectToIJKCoordinates(const SbVec3f & objectpos,
                                 const SbVec3f & volumesize,
                                 const SbVec3f & mincorner,
                                 const SbVec3s & voxeldimensions) const;

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

static SbBool
cvr_debug_raypicks(void)
{
  // If this environment flag is set, render the ray pick lines (for
  // debugging purposes).
  static int CVR_DEBUG_RAYPICKS = -1;
  if (CVR_DEBUG_RAYPICKS == -1) {
    const char * env = coin_getenv("CVR_DEBUG_RAYPICKS");
    CVR_DEBUG_RAYPICKS = env && (atoi(env) > 0);
  }
  return (CVR_DEBUG_RAYPICKS == 0) ? FALSE : TRUE;
}

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
  if (!this->shouldGLRender(action)) return;

  if (cvr_debug_raypicks()) { PRIVATE(this)->rayPickDebug(action); }

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

  SbVec3s dimensions;
  void * data;
  SoVolumeData::DataType type;
  SbBool ok = volumedata->getVolumeData(dimensions, data, type);
  assert(ok);

#if CVR_DEBUG && 0 // debug
  SoDebugError::postInfo("SoVolumeRender::GLRender",
                         "dimensions==[%d, %d, %d]",
                         dimensions[0], dimensions[1], dimensions[2]);
#endif // debug

  // FIXME: should check that rendering should be done through 2D
  // texture slices. 20021124 mortene.
  if (1) {
    if (!PRIVATE(this)->pagehandler) {
      PRIVATE(this)->pagehandler =
        new CvrPageHandler(dimensions, volumedata->getReader());
      // FIXME: the pagehandler instance must be devalidated /
      // destructed when data changes, the transferfunction changes or
      // other volume-rendering options that should cause
      // texture-updating changes. 20021124 mortene.
    }

    int numslices = PRIVATE(this)->calculateNrOfSlices(action, dimensions);
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
}

// doc in super
void
SoVolumeRender::rayPick(SoRayPickAction * action)
{
  if (!this->shouldRayPick(action)) return;

  SoState * state = action->getState();
  const SoVolumeDataElement * volumedataelement =
    SoVolumeDataElement::getInstance(state);
  if (volumedataelement == NULL) return;
  const SoVolumeData * volumedata = volumedataelement->getVolumeData();
  assert(volumedata != NULL);

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

  if (cvr_debug_raypicks()) {
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

  SbVec3s voxeldimensions;
  void * voxelptr;
  SoVolumeData::DataType voxeltype;
  SbBool ok = volumedata->getVolumeData(voxeldimensions, voxelptr, voxeltype);
  assert(ok);

  // Find objectspace-dimensions of a voxel.
  const SbVec3f size = maxcorner - mincorner;
  const SbVec3f voxelsize(voxeldimensions[0] / size[0],
                          voxeldimensions[1] / size[1],
                          voxeldimensions[2] / size[2]);

  // Calculate maximum number of voxels that could possibly be touched
  // by the ray.
  const SbVec3f rayvec = (intersects[1] - intersects[0]);
  const float minvoxdim = SbMin(voxelsize[0], SbMin(voxelsize[1], voxelsize[2]));
  const unsigned int maxvoxinray = (unsigned int)(rayvec.length() / minvoxdim + 1);

  const SbVec3f stepvec = rayvec / maxvoxinray;
  SbVec3s ijk, lastijk(-1, -1, -1);

  const SbBox3s voxelbounds(SbVec3s(0, 0, 0),
                            voxeldimensions - SbVec3s(1, 1, 1));

  SoPickedPoint * pp = NULL;
  SoVolumeRenderDetail * detail = NULL;
  CvrCLUT * clut = NULL;
  SbVec3f objectcoord = intersects[0];
  while (TRUE) {
    ijk = PRIVATE(this)->objectToIJKCoordinates(objectcoord, size, mincorner,
                                                voxeldimensions);
    if (!voxelbounds.intersect(ijk)) break;

    if (!action->isBetweenPlanes(objectcoord)) {
      objectcoord += stepvec;
      continue;
    }

    if (ijk != lastijk) { // touched new voxel
      lastijk = ijk;
      if (cvr_debug_raypicks()) {
        SoDebugError::postInfo("SoVolumeRender::rayPick",
                               "ijk=<%d, %d, %d>", ijk[0], ijk[1], ijk[2]);
      }
      if (pp == NULL) {
        pp = action->addIntersection(objectcoord);
        // if NULL, something else is obstructing the view to the
        // volume, the app programmer only want the nearest, and we
        // don't need to continue our intersection tests
        if (pp == NULL) return;
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
    else if (cvr_debug_raypicks()) {
      SoDebugError::postInfo("SoVolumeRender::rayPick", "duplicate");
    }

    objectcoord += stepvec;
  }

  if (clut) clut->unref();
}

SbVec3s
SoVolumeRenderP::objectToIJKCoordinates(const SbVec3f & objectpos,
                                        const SbVec3f & volumesize,
                                        const SbVec3f & mincorner,
                                        const SbVec3s & voxeldimensions) const
{
  SbVec3s ijk;
  for (int i=0; i < 3; i++) {
    const float normcoord = (objectpos[i] - mincorner[i]) / volumesize[i];
    ijk[i] = short(normcoord * (voxeldimensions[i] - 1.0f));
  }
  return ijk;
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
  \typedef AbortCode SoVolumeRender::SoVolumeRenderAbortCB(int totalslices, int thisslice,  void * userdata)

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
