#include <VolumeViz/nodes/SoVolumeRender.h>

#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/system/gl.h>
#include <VolumeViz/elements/SoTransferFunctionElement.h>
#include <VolumeViz/elements/SoVolumeDataElement.h>
#include <VolumeViz/nodes/SoVolumeData.h>

// *************************************************************************

SO_NODE_SOURCE(SoVolumeRender);

// *************************************************************************

class SoVolumeRenderP {
public:
  SoVolumeRenderP(SoVolumeRender * master) {
    this->master = master;
  }

private:
  SoVolumeRender * master;
};


#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

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
  assert(volumedataelement != NULL); // FIXME: handle gracefully. 20021111 mortene.

  SoVolumeData * volumedata = volumedataelement->getVolumeData();

  // Fetching the current transfer function.
  const SoTransferFunctionElement * transferfunctionelement =
    SoTransferFunctionElement::getInstance(state);
  assert(transferfunctionelement != NULL); // FIXME: handle gracefully. 20021111 mortene.

  SoTransferFunction * transferfunction =
    transferfunctionelement->getTransferFunction();

  // Calculating a camvec from camera to center of object.
  const SbMatrix & mm = SoModelMatrixElement::get(state);
  SbMatrix imm = mm.inverse();

  SbVec3f camvec;
  const SbViewVolume & vv = SoViewVolumeElement::get(state);

  if (/* FIXME: ??? 20021111 mortene: */ 0 &&
      vv.getProjectionType() == SbViewVolume::PERSPECTIVE) {
    SbVec3f worldpos(0.0f, 0.0f, 0.0f);
    mm.multVecMatrix(worldpos, worldpos);
    camvec = vv.getProjectionPoint() - worldpos;
    imm.multDirMatrix(camvec, camvec);
  }
  else { // ORTHOGRAPHIC
    camvec = - vv.getProjectionDirection();
    imm.multDirMatrix(camvec, camvec);
  }

  SbVec3f abstoviewer;
  abstoviewer[0] = fabs(camvec[0]);
  abstoviewer[1] = fabs(camvec[1]);
  abstoviewer[2] = fabs(camvec[2]);

  // Commonly used variables
  SbVec3f volmin, volmax;
  SbBox3f volumeSize = volumedata->getVolumeSize();
  volumeSize.getBounds(volmin, volmax);

#if CVR_DEBUG && 1 // debug
  SoDebugError::postInfo("SoVolumeRender::GLRender",
                         "volumeSize==[%f, %f, %f]",
                         volmax[0] - volmin[0],
                         volmax[1] - volmin[1],
                         volmax[2] - volmin[2]);
#endif // debug


  SbVec3s dimensions;
  void * data;
  SoVolumeData::DataType type;
  SbBool ok = volumedata->getVolumeData(dimensions, data, type);
  assert(ok);

#if CVR_DEBUG && 1 // debug
  SoDebugError::postInfo("SoVolumeRender::GLRender",
                         "dimensions==[%d, %d, %d]",
                         dimensions[0], dimensions[1], dimensions[2]);
#endif // debug

  SbBool renderalongX =
    (abstoviewer[0] >= abstoviewer[1]) &&
    (abstoviewer[0] >= abstoviewer[2]);

  SbBool renderalongY =
    (abstoviewer[1] >= abstoviewer[0]) &&
    (abstoviewer[1] >= abstoviewer[2]);

  SbBool renderalongZ =
    (abstoviewer[2] >= abstoviewer[0]) &&
    (abstoviewer[2] >= abstoviewer[1]);

  assert(((renderalongX ? 1 : 0) +
          (renderalongY ? 1 : 0) +
          (renderalongZ ? 1 : 0)) == 1);

  enum Axis { X = 0, Y = 1, Z = 2 };
  const int AXISIDX = (renderalongX ? X : (renderalongY ? Y : Z));

  int numslices = 0;
  int control = this->numSlicesControl.getValue();

  if (control == ALL) {
    numslices = dimensions[AXISIDX];
  }
  else if (control == MANUAL) {
    numslices = this->numSlices.getValue();
  }
  else if (control == AUTOMATIC) {
    float complexity = this->getComplexityValue(action);
    numslices = int(complexity * 2.0f * this->numSlices.getValue());
    assert(numslices >= 0);
  }
  else {
    assert(FALSE && "invalid numSlicesControl value");
  }

#if CVR_DEBUG && 1 // debug
  SoDebugError::postInfo("SoVolumeRender::GLRender",
                         "numslices == %d", numslices);
#endif // debug


  if (numslices == 0) return;

  float depth;
  float depthAdder;

  // Render in reverse order?
  if (camvec[AXISIDX] < 0)  {
    depthAdder = -(volmax[AXISIDX] - volmin[AXISIDX]) / numslices;
    depth = volmax[AXISIDX];
  }
  else {
    depthAdder = (volmax[AXISIDX] - volmin[AXISIDX]) / numslices;
    depth = volmin[AXISIDX];
  }

  // FIXME: is it really correct to use same quad for both X-way and
  // Y-way rendering? Seems bogus. 20021111 mortene.
  const SbBox2f QUAD = renderalongZ ?
    SbBox2f(volmin[0], volmin[1], volmax[0], volmax[1]) :
    SbBox2f(volmin[1], volmin[2], volmax[1], volmax[2]);

  const SbBox2f TEXTURECOORDS = SbBox2f(0.0, 0.0, 1.0, 1.0);

  glPushAttrib(GL_ALL_ATTRIB_BITS);

  glDisable(GL_LIGHTING);
  glEnable(GL_TEXTURE_2D);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  // FIXME: this is a reversion of a change that pederb made on
  // 20021104 that made the CoinVol/testcode/example program fail
  // (nothing gets drawn). Need to check with pederb what he tried to
  // accomplish with the change. (The log message says "Switched to
  // alpha test rendering instead of blending.") 20021109 mortene.
#if 1
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#else
  // FIXME: add proper transparency test. For now I've just disabled
  // blending and enabled glAlphaTest instead (looks better, and delayed
  // rendering is not required). pederb, 2002-11-04
  // glEnable(GL_BLEND);
  // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // this is to enable alpha test
  glAlphaFunc(GL_GREATER, 0.5f);
  glEnable(GL_ALPHA_TEST);
#endif

  glDisable(GL_CULL_FACE);

  // Rendering slices

  const int lastpageidx =
    (int)((float(numslices - 1) / float(numslices)) * dimensions[AXISIDX]);

  for (int i = 0; i < numslices; i++) {
    int pageidx =
      (int)((float(i)/float(numslices)) * float(dimensions[AXISIDX]));
    // If rendering in reverse order.
    if (depthAdder < 0) { pageidx = lastpageidx - pageidx; }

    volumedata->renderOrthoSlice(state, QUAD, depth, pageidx, TEXTURECOORDS,
                                 transferfunction, AXISIDX);
    depth += depthAdder;
  }

  glPopAttrib();
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
