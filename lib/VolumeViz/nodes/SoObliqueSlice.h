/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/

#ifndef COIN_SOOBLIQUESLICE_H
#define COIN_SOOBLIQUESLICE_H

#include <Inventor/nodes/SoNode.h>


class SoObliqueSlice : public SoShape
{
  typedef SoShape inherited;

  SO_NODE_HEADER(SoObliqueSlice);

public:
  static void initClass(void);

  enum Interpolation {
    NEAREST, 
    LINEAR
  };

  enum AlphaUse {
    ALPHA_AS_IS, 
    ALPHA_OPAQUE, 
    ALPHA_BINARY
  };

  // Fields
  SoSFPlane plane;
  SoSFEnum interpolation;
  SoSFEnum alphaUse;

  // Functions
  virtual SoType getTypeId() const;
  SoObliqueSlice();
  static SoType getClassTypeId();
};//SoObliqueSlice

#endif // !COIN_SOOBLIQUESLICE_H
