/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/


#include <VolumeViz/nodes/SoVolumeRender.h>
#include <VolumeViz/nodes/SoVolumeRendering.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <VolumeViz/elements/SoVolumeDataElement.h>
#include <Inventor/misc/SoGLImage.h>
#include <VolumeViz/nodes/SoVolumeData.h>
#include <Inventor/elements/SoGLCacheContextElement.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>

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
}//Constructor






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
  static int first = 0;
  if (first == 1) return;
  first = 1;

  SO_NODE_INIT_CLASS(SoVolumeRender, SoShape, "Shape");
}// initClass


void 
SoVolumeRender::generatePrimitives(SoAction * action)
{}


void 
SoVolumeRender::computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center)
{
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

  // ---- Calculating a camvec from camera to center of object
  const SbMatrix & mm = SoModelMatrixElement::get(state);
  SbMatrix imm = mm.inverse();

  SbVec3f camvec;
  const SbViewVolume & vv = SoViewVolumeElement::get(state);

  if (0 && vv.getProjectionType() == SbViewVolume::PERSPECTIVE) {
    SbVec3f worldpos(0.0f, 0.0f, 0.0f);
    mm.multVecMatrix(worldpos, worldpos);
    camvec = vv.getProjectionPoint() - worldpos;
    imm.multDirMatrix(camvec, camvec);
  }// if
  else { // ORTHOGRAPHIC
    camvec = - vv.getProjectionDirection();
    imm.multDirMatrix(camvec, camvec);
  }// if

  SbVec3f abstoviewer;
  abstoviewer[0] = fabs(camvec[0]);
  abstoviewer[1] = fabs(camvec[1]);
  abstoviewer[2] = fabs(camvec[2]);

  glPushAttrib(GL_ALL_ATTRIB_BITS);

  glDisable(GL_LIGHTING);
  glEnable(GL_TEXTURE_2D);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);

  // FIXME: Implement use of the 
  // numSlicesControl-field. torbjorv 07112002

  // FIXME: Clean up the use of states for textures, lighting,
  // culling, blendfunc (restore previous states). 
  // torbjorv 07122002


  // Commonly used variables
  SbVec3f min, max;
  SbBox3f volumeSize = volumeData->getVolumeSize();
  volumeSize.getBounds(min, max);
  SbVec3s dimensions = volumeData->getDimensions();
  float depth;
  float depthAdder;


  // ---- Render along X-axis
  if ((abstoviewer[0] >= abstoviewer[1]) &&
      (abstoviewer[0] >= abstoviewer[2])) {

    // Render in reverse order?
    if (camvec[0] < 0)  {
      depthAdder = -(max[0] - min[0])/numSlices.getValue();
      depth = max[0];
    }//if
    else {
      depthAdder = (max[0] - min[0])/numSlices.getValue();
      depth = min[0];
    }//else


    // Rendering slices
    for (int i = 0; i < numSlices.getValue(); i++) {
      int imageIdx = (int)((float(i)/float(numSlices.getValue()))*float(dimensions[0]));

      // Are we rendering in in reverse order?
      if (depthAdder < 0)
        imageIdx = dimensions[0] - 1 - imageIdx;

      SoGLImage * image = volumeData->getGLImageSlice(imageIdx, SoVolumeData::X);
      SoGLDisplayList * dl = image->getGLDisplayList(state);
      dl->call(state);

      glBegin(GL_QUADS);
      glColor4f(1, 1, 1, 1);
      glTexCoord2f(0.0f, 0.0f);
      glVertex3f(depth, min[1], min[2]);    
      glTexCoord2f(1.0f, 0.0f);
      glVertex3f(depth, min[1], max[2]);    
      glTexCoord2f(1.0f, 1.0f);
      glVertex3f(depth, max[1], max[2]);    
      glTexCoord2f(0.0f, 1.0f);
      glVertex3f(depth, max[1], min[2]);    
      depth += depthAdder;

      glEnd();
    }// for
  }// if
  else 



  // ---- Render along Y-axis
  if ((abstoviewer[1] >= abstoviewer[0]) &&
      (abstoviewer[1] >= abstoviewer[2])) {

    // Render in reverse order?
    if (camvec[1] < 0)  {
      depthAdder = -(max[1] - min[1])/numSlices.getValue();
      depth = max[1];
    }//if
    else {
      depthAdder = (max[1] - min[1])/numSlices.getValue();
      depth = min[1];
    }//if

    // Rendering slices
    for (int i = 0; i < numSlices.getValue(); i++) {
      int imageIdx = 
        (int)((float(i)/float(numSlices.getValue()))*float(dimensions[1]));

      // Are we rendering in in reverse order?
      if (depthAdder < 0)
        imageIdx = dimensions[1] - 1 - imageIdx;

      SoGLImage * image = 
        volumeData->getGLImageSlice(imageIdx, SoVolumeData::Y);
      SoGLDisplayList * dl = image->getGLDisplayList(state);
      dl->call(state);

      glBegin(GL_QUADS);
      glColor4f(1, 1, 1, 1);
      glTexCoord2f(0.0f, 1.0f);
      glVertex3f(min[0], depth, min[2]);    
      glTexCoord2f(1.0f, 1.0f);
      glVertex3f(max[0], depth, min[2]);    
      glTexCoord2f(1.0f, 0.0f);
      glVertex3f(max[0], depth, max[2]);    
      glTexCoord2f(0.0f, 0.0f);
      glVertex3f(min[0], depth, max[2]);    
      depth += depthAdder;

      glEnd();
    }// for
  }// else if
  else 




  // ---- Render along Z-axis
  if ((abstoviewer[2] >= abstoviewer[0]) &&
      (abstoviewer[2] >= abstoviewer[1])) {

    // Render in reverse order?
    if (camvec[2] < 0)  {
      depthAdder = -(max[2] - min[2])/numSlices.getValue();
      depth = max[2];
    }//if
    else {
      depthAdder = +(max[2] - min[2])/numSlices.getValue();
      depth = min[2];
    }//if

    // Rendering slices
    for (int i = 0; i < numSlices.getValue(); i++) {
      int imageIdx 
        = (int)((float(i)/float(numSlices.getValue()))*float(dimensions[2]));

      // Are we rendering in in reverse order?
      if (camvec[2] < 0)
        imageIdx = dimensions[2] - 1 - imageIdx;

      SoGLImage * image = 
        volumeData->getGLImageSlice(imageIdx, SoVolumeData::Z);
      SoGLDisplayList * dl = image->getGLDisplayList(state);
      dl->call(state);

      glBegin(GL_QUADS);
      glColor4f(1, 1, 1, 1);
      glTexCoord2f(0.0f, 0.0f);
      glVertex3f(min[0], min[1], depth);    
      glTexCoord2f(1.0f, 0.0f);
      glVertex3f(max[0], min[1], depth);    
      glTexCoord2f(1.0f, 1.0f);
      glVertex3f(max[0], max[1], depth);    
      glTexCoord2f(0.0f, 1.0f);
      glVertex3f(min[0], max[1], depth);    
      depth += depthAdder;

      glEnd();
    }// for
  }// else if

  glPopAttrib();
}// GLRender
