#ifndef COIN_SOORTHOSLICE_H
#define COIN_SOORTHOSLICE_H

#include <Inventor/nodes/SoNode.h>


class SoOrthoSlice : public SoShape {
  typedef SoShape inherited;

  SO_NODE_HEADER(SoOrthoSlice);

public:
  static void initClass(void);

  enum Axis { X, Y, Z };
  enum Interpolation { NEAREST, LINEAR };
  enum AlphaUse { ALPHA_AS_IS, ALPHA_OPAQUE, ALPHA_BINARY };
  enum ClippingSide { FRONT, BACK };

  SoOrthoSlice(void);
  virtual SbBool affectsState(void) const;

  SoSFUInt32 sliceNumber;
  SoSFEnum axis;
  SoSFEnum interpolation;
  SoSFEnum alphaUse;
  SoSFEnum clippingSide;
  SoSFBool clipping;
};

#endif // !COIN_SOORTHOSLICE_H
