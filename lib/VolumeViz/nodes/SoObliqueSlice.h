#ifndef COIN_SOOBLIQUESLICE_H
#define COIN_SOOBLIQUESLICE_H

#include <Inventor/nodes/SoNode.h>


class SoObliqueSlice : public SoShape {
  typedef SoShape inherited;

  SO_NODE_HEADER(SoObliqueSlice);

public:
  static void initClass(void);

  enum Interpolation { NEAREST, LINEAR };
  enum AlphaUse { ALPHA_AS_IS, ALPHA_OPAQUE, ALPHA_BINARY };

  SoSFPlane plane;
  SoSFEnum interpolation;
  SoSFEnum alphaUse;

  SoObliqueSlice();
};

#endif // !COIN_SOOBLIQUESLICE_H
