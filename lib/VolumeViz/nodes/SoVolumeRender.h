/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/

#ifndef COIN_SOVOLUMERENDER_H
#define COIN_SOVOLUMERENDER_H

#include <Inventor/nodes/SoNode.h>
#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/nodes/SoShape.h>
#include <Inventor/fields/SoSFEnum.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFVec3f.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoSFInt32.h>



class SoVolumeRender : public SoShape {
  typedef SoShape inherited;

  SO_NODE_HEADER(SoVolumeRender);

public:
  static void initClass(void);

  enum Interpolation {
    NEAREST, 
    LINEAR
  };

  enum Composition {
    MAX_INTENSITY, 
    SUM_INTENSITY, 
    ALPHA_BLENDING
  };

  enum NumSlicesControl {
    ALL, 
    MANUAL, 
    AUTOMATIC
  };

  enum AbortCode {
    CONTINUE, 
    ABORT, 
    SKIP
  };

  typedef AbortCode SoVolumeRenderAbortCB(int totalSlices, int thisSlice, void *userData);

  // Fields
  SoSFEnum interpolation;
  SoSFEnum composition;
  SoSFBool lighting;
  SoSFVec3f lightDirection;
  SoSFFloat lightIntensity;
  SoSFEnum numSlicesControl;
  SoSFInt32 numSlices;
  SoSFBool viewAlignedSlices;

  // Functions
  SoVolumeRender();
  ~SoVolumeRender();
  void setAbortCallback (SoVolumeRenderAbortCB *func, void *userData=NULL);

protected:
  virtual void GLRender(SoGLRenderAction *action);
  virtual void generatePrimitives(SoAction * action);
  virtual void computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center);


private:
  friend class SoVolumeRenderP;
  class SoVolumeRenderP * pimpl;


};//SoVolumeRender

#endif // !COIN_SOVOLUMERENDER_H
