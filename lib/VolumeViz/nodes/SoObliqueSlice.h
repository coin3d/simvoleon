#ifndef COIN_SOOBLIQUESLICE_H
#define COIN_SOOBLIQUESLICE_H

#include <Inventor/nodes/SoShape.h>
#include <Inventor/fields/SoSFPlane.h>
#include <Inventor/fields/SoSFEnum.h>
#include <VolumeViz/C/basic.h>


class SIMVOLEON_DLL_API SoObliqueSlice : public SoShape {
  typedef SoShape inherited;

  SO_NODE_HEADER(SoObliqueSlice);

public:
  static void initClass(void);
  SoObliqueSlice(void);

  enum Interpolation { NEAREST, LINEAR };
  enum AlphaUse { ALPHA_AS_IS, ALPHA_OPAQUE, ALPHA_BINARY };

  SoSFPlane plane;
  SoSFEnum interpolation;
  SoSFEnum alphaUse;

protected:
  ~SoObliqueSlice();

  virtual void GLRender(SoGLRenderAction * action);
  virtual void rayPick(SoRayPickAction * action);
  virtual void generatePrimitives(SoAction * action);
  virtual void computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center);

private:
  friend class SoObliqueSliceP;
  class SoObliqueSliceP * pimpl;
};

#endif // !COIN_SOOBLIQUESLICE_H
