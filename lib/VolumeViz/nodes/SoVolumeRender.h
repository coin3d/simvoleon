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
  SoVolumeRender(void);

  enum Interpolation { NEAREST, LINEAR };
  enum Composition { MAX_INTENSITY, SUM_INTENSITY, ALPHA_BLENDING };
  enum NumSlicesControl { ALL, MANUAL, AUTOMATIC };

  enum AbortCode { CONTINUE, ABORT, SKIP };
  typedef AbortCode SoVolumeRenderAbortCB(int totalslices, int thisslice, 
                                          void * userdata);

  void setAbortCallback(SoVolumeRenderAbortCB * func, void * userdata = NULL);

  SoSFEnum interpolation;
  SoSFEnum composition;
  SoSFBool lighting;
  SoSFVec3f lightDirection;
  SoSFFloat lightIntensity;
  SoSFEnum numSlicesControl;
  SoSFInt32 numSlices;
  SoSFBool viewAlignedSlices;

protected:
  ~SoVolumeRender();

  virtual void GLRender(SoGLRenderAction * action);
  virtual void generatePrimitives(SoAction * action);
  virtual void computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center);

private:
  friend class SoVolumeRenderP;
  class SoVolumeRenderP * pimpl;
};

#endif // !COIN_SOVOLUMERENDER_H
