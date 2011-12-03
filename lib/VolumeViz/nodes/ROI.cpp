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

// FIXME: not yet properly implemented. 20040505 mortene.


/*
FIXME

  SoROIP::boxes keeps all the boxes to be rendered. A ROI may be specified
  using a number of clipping planes, resulting in a volume impossible to 
  describe with one box. SoROI::GLRender renders all the boxes currently 
  stored in SoROIP::boxes, but does NOT depthsort them first. This is 
  crucial, and must be implemented. A sensor and a callback is attached to
  SoROI::box and should generate a new list of boxes whenever this field
  changes. It should also be attached to SoROI::subVolume. 

  torbjorv 08282002
*/


#include <VolumeViz/nodes/SoROI.h>

#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/sensors/SoFieldSensor.h>
#include <Inventor/system/gl.h>
#include <VolumeViz/elements/SoTransferFunctionElement.h>
#include <VolumeViz/nodes/SoVolumeRender.h>
#include <VolumeViz/nodes/SoVolumeRendering.h>

// *************************************************************************

SO_NODE_SOURCE(SoROI);

// *************************************************************************

class SoROIP{
public:
  SoROIP(SoROI * master) {
    this->master = master;
    boxes = NULL;
    numBoxes = 0;
  }

  static void boxCallback(void *data, SoSensor *sensor);

  int numBoxes;
  SbBox3s *boxes;
  SbVec3s dim;

  SoFieldSensor * boxSensor;
private:
  SoROI * master;
};


#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

// *************************************************************************

SoROI::SoROI(void)
{
  SO_NODE_CONSTRUCTOR(SoROI);

  PRIVATE(this) = new SoROIP(this);

  SO_NODE_DEFINE_ENUM_VALUE(Flags, ENABLE_X0);
  SO_NODE_DEFINE_ENUM_VALUE(Flags, ENABLE_Y0);
  SO_NODE_DEFINE_ENUM_VALUE(Flags, ENABLE_Z0);
  SO_NODE_DEFINE_ENUM_VALUE(Flags, INVERT_0);
  SO_NODE_DEFINE_ENUM_VALUE(Flags, ENABLE_X1);
  SO_NODE_DEFINE_ENUM_VALUE(Flags, ENABLE_Y1);
  SO_NODE_DEFINE_ENUM_VALUE(Flags, ENABLE_Z1);
  SO_NODE_DEFINE_ENUM_VALUE(Flags, INVERT_1);
  SO_NODE_DEFINE_ENUM_VALUE(Flags, ENABLE_X2);
  SO_NODE_DEFINE_ENUM_VALUE(Flags, ENABLE_Y2);
  SO_NODE_DEFINE_ENUM_VALUE(Flags, ENABLE_Z2);
  SO_NODE_DEFINE_ENUM_VALUE(Flags, INVERT_2);
  SO_NODE_DEFINE_ENUM_VALUE(Flags, OR_SELECT);
  SO_NODE_DEFINE_ENUM_VALUE(Flags, INVERT_OUTPUT);
  SO_NODE_DEFINE_ENUM_VALUE(Flags, SUB_VOLUME);
  SO_NODE_DEFINE_ENUM_VALUE(Flags, EXCLUSION_BOX);
  SO_NODE_DEFINE_ENUM_VALUE(Flags, CROSS);
  SO_NODE_DEFINE_ENUM_VALUE(Flags, CROSS_INVERT);
  SO_NODE_DEFINE_ENUM_VALUE(Flags, FENCE);
  SO_NODE_DEFINE_ENUM_VALUE(Flags, FENCE_INVERT);
  SO_NODE_SET_SF_ENUM_TYPE(flags, Flags);

  SO_NODE_ADD_FIELD(relative, (FALSE)); 
  SO_NODE_ADD_FIELD(flags, (SUB_VOLUME)); 
  SO_NODE_ADD_FIELD(box, (0, 0, 0, 1, 1, 1));
  SO_NODE_ADD_FIELD(subVolume, (0, 0, 0, 0, 0, 0));

  PRIVATE(this)->boxSensor = new SoFieldSensor(PRIVATE(this)->boxCallback, this);
  PRIVATE(this)->boxSensor->attach(&this->box);
}

SoROI::~SoROI()
{
  delete PRIVATE(this);
}

// Doc from parent class.
void
SoROI::initClass(void)
{
  SO_NODE_INIT_CLASS(SoROI, SoVolumeRendering, "SoVolumeRendering");

  SO_ENABLE(SoGLRenderAction, SoTransferFunctionElement);
  SO_ENABLE(SoGLRenderAction, SoVolumeDataElement);
}


// FIXME: try to merge this code with the SoVolumeRender::GLRender()
// code. 20021111 mortene.
void 
SoROI::GLRender(SoGLRenderAction *action)
{
#if 1
  SoDebugError::postWarning("SoROI::GLRender",
                            "not implemented yet");
#else
  // FIXME: now defunct, due to reorganization and refactoring of
  // interfaces this depends on. The code never really worked, though,
  // so no big loss.
  //
  // When implementing "proper", should probably share code with
  // SoVolumeRender node's GLRender(). 20021121 mortene.

  SoState * state = action->getState();

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
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);

  // FIXME: Clean up the use of states for textures, lighting,
  // culling, blendfunc (restore previous states). 
  // torbjorv 07122002

  // Commonly used variables
  SbVec3f volumeMin, volumeMax;
  SbBox3f volumeSize = volumeData->getVolumeSize();
  volumeSize.getBounds(volumeMin, volumeMax);

  SbVec3s dimensions;
  void * data;
  SoVolumeData::DataType type;
  SbBool ok = volumeData->getVolumeData(dimensions, data, type);
  assert(ok);

  SbVec3s minSlice, maxSlice;
  SbBox3s sliceBox;
  this->box.getValue(sliceBox);
  sliceBox.getBounds(minSlice, maxSlice);
  SbVec3f min, max;
  min[0] = 
    volumeMin[0] + 
    (volumeMax[0] - volumeMin[0])*(float(minSlice[0]) / dimensions[0]);
  min[1] = 
    volumeMin[1] + 
    (volumeMax[1] - volumeMin[1])*(float(minSlice[1]) / dimensions[1]);
  min[2] = 
    volumeMin[2] + 
    (volumeMax[2] - volumeMin[2])*(float(minSlice[2]) / dimensions[2]);
  max[0] = 
    volumeMin[0] + 
    (volumeMax[0] - volumeMin[0])*(float(maxSlice[0]) / dimensions[0]);
  max[1] = 
    volumeMin[1] + 
    (volumeMax[1] - volumeMin[1])*(float(maxSlice[1]) / dimensions[1]);
  max[2] = 
    volumeMin[2] + 
    (volumeMax[2] - volumeMin[2])*(float(maxSlice[2]) / dimensions[2]);

  float depth;
  float depthAdder;

  for (int i = 0; i < PRIVATE(this)->numBoxes; i++) {
    // ---- Render along X-axis
    if ((abstoviewer[0] >= abstoviewer[1]) &&
        (abstoviewer[0] >= abstoviewer[2])) {
      int numSlices = maxSlice[0] - minSlice[0];

      // Render in reverse order?
      if (camvec[0] < 0)  {
        depthAdder = -(max[0] - min[0])/numSlices;
        depth = max[0];
      }
      else {
        depthAdder = (max[0] - min[0])/numSlices;
        depth = min[0];
      }

      SbBox2f mappingCoords = SbBox2f(float(minSlice[2])/dimensions[2],
                                      float(minSlice[1])/dimensions[1],
                                      float(maxSlice[2])/dimensions[2],
                                      float(maxSlice[1])/dimensions[1]);

      // Rendering slices
      for (int i = minSlice[0]; i < maxSlice[0]; i++) {
        int imageIdx = i;

        // Are we rendering in in reverse order?
        if (camvec[0] < 0)
          imageIdx = maxSlice[0] - (i - minSlice[0]) - 1;

        volumeData->renderOrthoSlice(state,
                                     SbBox2f(min[2], 
                                             min[1], 
                                             max[2], 
                                             max[1]), 
                                     depth,
                                     imageIdx,
                                     mappingCoords,
                                     transferFunction,
                                     0 /* axis */);

        depth += depthAdder;
      }
    }
    else 


 
    // ---- Render along Y-axis
    if ((abstoviewer[1] >= abstoviewer[0]) &&
        (abstoviewer[1] >= abstoviewer[2])) {
      int numSlices = maxSlice[1] - minSlice[1];

      // Render in reverse order?
      if (camvec[1] < 0)  {
        depthAdder = -(max[1] - min[1])/numSlices;
        depth = max[1];
      }
      else {
        depthAdder = (max[1] - min[1])/numSlices;
        depth = min[1];
      }

      SbBox2f mappingCoords = SbBox2f(float(minSlice[0])/dimensions[0],
                                      float(minSlice[2])/dimensions[2],
                                      float(maxSlice[0])/dimensions[0],
                                      float(maxSlice[2])/dimensions[2]);

      // Rendering slices
      for (int i = minSlice[1]; i < maxSlice[1]; i++) {
        int imageIdx = i;

        // Are we rendering in in reverse order?
        if (camvec[1] < 0)
          imageIdx = maxSlice[1] - (i - minSlice[1]) - 1;

        volumeData->renderOrthoSlice(state,
                                     SbBox2f(min[0], 
                                             min[2], 
                                             max[0], 
                                             max[2]), 
                                     depth,
                                     imageIdx,
                                     mappingCoords,
                                     transferFunction,
                                     1 /* axis */);

        depth += depthAdder;
      }
    }
    else 




    // ---- Render along Z-axis
    if ((abstoviewer[2] >= abstoviewer[0]) &&
        (abstoviewer[2] >= abstoviewer[1])) {
      int numSlices = maxSlice[2] - minSlice[2];

      // Render in reverse order?
      if (camvec[2] < 0)  {
        depthAdder = -(max[2] - min[2])/numSlices;
        depth = max[2];
      }
      else {
        depthAdder = +(max[2] - min[2])/numSlices;
        depth = min[2];
      }

      SbBox2f mappingCoords = SbBox2f(float(minSlice[0])/dimensions[0],
                                      float(minSlice[1])/dimensions[1],
                                      float(maxSlice[0])/dimensions[0],
                                      float(maxSlice[1])/dimensions[1]);

      // Rendering slices
      for (int i = minSlice[2]; i < maxSlice[2]; i++) {
        int imageIdx = i;

        // Are we rendering in in reverse order?
        if (camvec[2] < 0)
          imageIdx = maxSlice[2] - (i - minSlice[2]) - 1;

        volumeData->renderOrthoSlice(state,
                                     SbBox2f(min[0], 
                                             min[1], 
                                             max[0], 
                                             max[1]), 
                                     depth,
                                     imageIdx,
                                     mappingCoords,
                                     transferFunction,
                                     2 /* axis */);

        depth += depthAdder;
      }
    }
  }

  glPopAttrib();
#endif
}


// FIXME: Implement these functions... torbjorv 07312002
void SoROI::doAction(SoAction *action) {}
void SoROI::callback(SoCallbackAction *action) {}
void SoROI::getBoundingBox(SoGetBoundingBoxAction *action) {}
void SoROI::pick(SoPickAction *action) {}


/*************************** PIMPL-FUNCTIONS ********************************/

void
SoROIP::boxCallback(void *data, SoSensor *sensor)
{
  SoROI * thisp = (SoROI *)data;

  // FIXME: Generate all boxes. This is just a dummybox. torbjorv 08012002
  // FIXME: Check if the box's coordinates are valid. Clamp if not. 
  // torbjorv 08012002

  // Generating all visible boxes
  delete [] PRIVATE(thisp)->boxes;
  PRIVATE(thisp)->boxes = new SbBox3s[1];
  PRIVATE(thisp)->numBoxes = 1;

  thisp->box.getValue(PRIVATE(thisp)->boxes[0]);
}
