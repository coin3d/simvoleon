/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/

#ifndef COIN_SOORTHOSLICE_H
#define COIN_SOORTHOSLICE_H

#include <Inventor/nodes/SoNode.h>


class SoOrthoSlice : public SoShape {
  typedef SoShape inherited;

  SO_NODE_HEADER(SoOrthoSlice);

public:
  static void initClass(void);

  enum Axis {
    X, 
    Y, 
    Z
  };

  enum Interpolation {
    NEAREST, 
    LINEAR
  };

  enum AlphaUse {
    ALPHA_AS_IS, 
    ALPHA_OPAQUE, 
    ALPHA_BINARY
  };

  enum ClippingSide {
    FRONT, 
    BACK
  };

  // Functions
  virtual SoType getTypeId () const;
  SoOrthoSlice ();
  virtual SbBool affectsState () const;
  static SoType getClassTypeId ();

  // Fields
  SoSFUInt32 sliceNumber;
  SoSFEnum axis;
  SoSFEnum interpolation;
  SoSFEnum alphaUse;
  SoSFEnum clippingSide;
  SoSFBool clipping;
};//SoOrthoSlice

#endif // !COIN_SOORTHOSLICE_H
