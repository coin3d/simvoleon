#ifndef COIN_SOORTHOSLICE_H
#define COIN_SOORTHOSLICE_H

#include <Inventor/nodes/SoShape.h>
#include <Inventor/fields/SoSFUInt32.h>
#include <Inventor/fields/SoSFEnum.h>
#include <Inventor/fields/SoSFBool.h>


class SoOrthoSlice : public SoShape {
  typedef SoShape inherited;

  SO_NODE_HEADER(SoOrthoSlice);

public:
  static void initClass(void);

  SoOrthoSlice(void);

  enum Axis { X = 0, Y, Z };
  enum Interpolation { NEAREST, LINEAR };
  enum AlphaUse { ALPHA_AS_IS, ALPHA_OPAQUE, ALPHA_BINARY };
  enum ClippingSide { FRONT, BACK };

  virtual SbBool affectsState(void) const;

  SoSFUInt32 sliceNumber;
  SoSFEnum axis;
  SoSFEnum interpolation;
  SoSFEnum alphaUse;
  SoSFEnum clippingSide;
  SoSFBool clipping;

protected:
  ~SoOrthoSlice();

  virtual void doAction(SoAction * action);
  virtual void GLRender(SoGLRenderAction * action);
  virtual void rayPick(SoRayPickAction * action);
  virtual void generatePrimitives(SoAction * action);
  virtual void computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center);

private:
  friend class SoOrthoSliceP;
  class SoOrthoSliceP * pimpl;
};

#endif // !COIN_SOORTHOSLICE_H
