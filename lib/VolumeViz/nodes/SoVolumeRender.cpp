#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/elements/SoGLCacheContextElement.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>

#include <VolumeViz/elements/SoVolumeDataElement.h>
#include <VolumeViz/nodes/SoVolumeData.h>
#include <VolumeViz/nodes/SoVolumeRender.h>
#include <VolumeViz/nodes/SoVolumeRendering.h>
#include <VolumeViz/elements/SoTransferFunctionElement.h>


#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif // HAVE_WINDOWS_H*/

#include <GL/gl.h>

// *************************************************************************

SO_NODE_SOURCE(SoVolumeRender);

// *************************************************************************

class SoVolumeRenderP{
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

/*!
  Constructor.
*/
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

  SO_NODE_DEFINE_ENUM_VALUE(AbortCode, CONTINUE);
  SO_NODE_DEFINE_ENUM_VALUE(AbortCode, ABORT);
  SO_NODE_DEFINE_ENUM_VALUE(AbortCode, SKIP);

  SO_NODE_ADD_FIELD(interpolation, (SoVolumeRender::LINEAR));
  SO_NODE_ADD_FIELD(composition, (SoVolumeRender::ALPHA_BLENDING));
  SO_NODE_ADD_FIELD(lighting, (FALSE));
  SO_NODE_ADD_FIELD(lightDirection, (SbVec3f(-1, -1, -1)));
  SO_NODE_ADD_FIELD(lightIntensity, (1.0));
  SO_NODE_ADD_FIELD(numSlicesControl, (SoVolumeRender::ALL));
  SO_NODE_ADD_FIELD(numSlices, (10));
  SO_NODE_ADD_FIELD(viewAlignedSlices, (FALSE));
}

/*!
  Destructor.
*/
SoVolumeRender::~SoVolumeRender()
{
  delete PRIVATE(this);
}




// Doc from parent class.
void
SoVolumeRender::initClass(void)
{
  SO_NODE_INIT_CLASS(SoVolumeRender, SoShape, "Shape");
}



// doc in super
void 
SoVolumeRender::GLRender(SoGLRenderAction *action)
{
  SoState * state = action->getState();
  if (!shouldGLRender(action)) return;

  // Fetching the current volumedata
  const SoVolumeDataElement * volumeDataElement;
  volumeDataElement = SoVolumeDataElement::getInstance(state);
  assert(volumeDataElement);
  SoVolumeData * volumeData = volumeDataElement->getVolumeData();

  // Fetching the current transferFunction
  const SoTransferFunctionElement * transferFunctionElement;
  transferFunctionElement = SoTransferFunctionElement::getInstance(state);
  assert(transferFunctionElement);
  SoTransferFunction * transferFunction = 
    transferFunctionElement->getTransferFunction();

  // Calculating a camvec from camera to center of object
  const SbMatrix & mm = SoModelMatrixElement::get(state);
  SbMatrix imm = mm.inverse();

  SbVec3f camvec;
  const SbViewVolume & vv = SoViewVolumeElement::get(state);

  if (0 && vv.getProjectionType() == SbViewVolume::PERSPECTIVE) {
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

  glPushAttrib(GL_ALL_ATTRIB_BITS);

  glDisable(GL_LIGHTING);
  glEnable(GL_TEXTURE_2D);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  // FIXME: add proper transparency test. For now I've just disabled
  // blending and enabled glAlphaTest instead (looks better, and delayed
  // rendering is not required). pederb, 2002-11-04
  // glEnable(GL_BLEND);
  // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // this is to enable alpha test
  glAlphaFunc(GL_GREATER, 0.5f);
  glEnable(GL_ALPHA_TEST);


  glDisable(GL_CULL_FACE);

  // FIXME: Implement use of the 
  // numSlicesControl-field. torbjorv 07112002

  // FIXME: Clean up the use of states for textures, lighting,
  // culling, blendfunc (restore previous states). 
  // torbjorv 07122002

  // FIXME: 


  // Commonly used variables
  SbVec3f min, max;
  SbBox3f volumeSize = volumeData->getVolumeSize();
  volumeSize.getBounds(min, max);
  SbVec3s dimensions = volumeData->getDimensions();
  float depth;
  float depthAdder;


  // Render along X-axis
  if ((abstoviewer[0] >= abstoviewer[1]) &&
      (abstoviewer[0] >= abstoviewer[2])) {

    // Render in reverse order?
    if (camvec[0] < 0)  {
      depthAdder = -(max[0] - min[0])/numSlices.getValue();
      depth = max[0];
    }
    else {
      depthAdder = (max[0] - min[0])/numSlices.getValue();
      depth = min[0];
    }


    // Rendering slices
    for (int i = 0; i < numSlices.getValue(); i++) {
      int imageIdx = 
        (int)((float(i)/float(numSlices.getValue()))*float(dimensions[0]));

      // Are we rendering in in reverse order?
      if (depthAdder < 0)
        imageIdx = 
          (int)((float(numSlices.getValue() - 1)/numSlices.getValue())*
          dimensions[0]) - 
          imageIdx;

      volumeData->renderOrthoSliceX(state,
                                    SbBox2f(min[1], 
                                            min[2], 
                                            max[1], 
                                            max[2]), 
                                    depth,
                                    imageIdx,
                                    SbBox2f(0.0, 0.0, 1.0, 1.0),
                                    transferFunction);

      depth += depthAdder;
    }
  }
  else 



  // Render along Y-axis
  if ((abstoviewer[1] >= abstoviewer[0]) &&
      (abstoviewer[1] >= abstoviewer[2])) {

    // Render in reverse order?
    if (camvec[1] < 0)  {
      depthAdder = -(max[1] - min[1])/numSlices.getValue();
      depth = max[1];
    }
    else {
      depthAdder = (max[1] - min[1])/numSlices.getValue();
      depth = min[1];
    }

    // Rendering slices
    for (int i = 0; i < numSlices.getValue(); i++) {
      int imageIdx = 
        (int)((float(i)/float(numSlices.getValue()))*float(dimensions[1]));

      // Are we rendering in in reverse order?
      if (depthAdder < 0)
        imageIdx = 
          (int)((float(numSlices.getValue() - 1)/numSlices.getValue())*
          dimensions[1]) - 
          imageIdx;

      volumeData->renderOrthoSliceY(state,
                                    SbBox2f(min[1], 
                                            min[2], 
                                            max[1], 
                                            max[2]), 
                                    depth,
                                    imageIdx,
                                    SbBox2f(0.0, 0.0, 1.0, 1.0),
                                    transferFunction);

      depth += depthAdder;
    }
  }
  else 




  // Render along Z-axis
  if ((abstoviewer[2] >= abstoviewer[0]) &&
      (abstoviewer[2] >= abstoviewer[1])) {

    // Render in reverse order?
    if (camvec[2] < 0)  {
      depthAdder = -(max[2] - min[2])/numSlices.getValue();
      depth = max[2];
    }
    else {
      depthAdder = +(max[2] - min[2])/numSlices.getValue();
      depth = min[2];
    }

    // Rendering slices
    for (int i = 0; i < numSlices.getValue(); i++) {
      int imageIdx 
        = (int)((float(i)/float(numSlices.getValue()))*float(dimensions[2]));

      // Are we rendering in in reverse order?
      if (camvec[2] < 0)
        imageIdx = 
          (int)((float(numSlices.getValue() - 1)/numSlices.getValue())*
          dimensions[2]) - 
          imageIdx;

      volumeData->renderOrthoSliceZ(state,
                                    SbBox2f(min[0], 
                                            min[1], 
                                            max[0], 
                                            max[1]), 
                                    depth,
                                    imageIdx,
                                    SbBox2f(0.0, 0.0, 1.0, 1.0),
                                    transferFunction);

      depth += depthAdder;
    }
  }

  glPopAttrib();
}

void 
SoVolumeRender::generatePrimitives(SoAction * action)
{
}


void 
SoVolumeRender::computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center)
{
}
