#include <VolumeViz/nodes/SoVolumeRender.h>

#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/system/gl.h>

#include <VolumeViz/elements/SoTransferFunctionElement.h>
#include <VolumeViz/elements/SoVolumeDataElement.h>
#include <VolumeViz/nodes/SoVolumeData.h>
#include <VolumeViz/render/2D/CvrPageHandler.h>

// *************************************************************************

SO_NODE_SOURCE(SoVolumeRender);

// *************************************************************************

class SoVolumeRenderP {
public:
  SoVolumeRenderP(SoVolumeRender * master)
  {
    this->master = master;
    this->pagehandler = NULL;
  }

  ~SoVolumeRenderP()
  {
    delete this->pagehandler;
  }

  unsigned int calculateNrOfSlices(SoGLRenderAction * action,
                                   const SbVec3s & dimensions);

  CvrPageHandler * pagehandler;

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
  SO_ENABLE(SoGLRenderAction, SoVolumeDataElement);
}



// doc in super
void
SoVolumeRender::GLRender(SoGLRenderAction * action)
{
  if (!this->shouldGLRender(action)) return;

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

    PRIVATE(this)->pagehandler->render(action, numslices);
  }
}

void
SoVolumeRender::generatePrimitives(SoAction * action)
{
  // FIXME: implement me. 20021120 mortene.
}


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
