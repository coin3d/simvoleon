/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/

#ifndef COIN_SOTRANSFERFUNCTION_H
#define COIN_SOTRANSFERFUNCTION_H

#include <VolumeViz/nodes/SoVolumeRendering.h>
#include <Inventor/fields/SoSFEnum.h>
#include <Inventor/fields/SoSFInt32.h>
#include <Inventor/fields/SoMFFloat.h>

class SoTransferFunction : public SoVolumeRendering {
  typedef SoVolumeRendering inherited;

  SO_NODE_HEADER(SoTransferFunction);

public:
  static void initClass(void);

  enum PredefColorMap {
    NONE, 
    GREY, 
    GRAY = GREY, 
    TEMPERATURE, 
    PHYSICS, 
    STANDARD, 
    GLOW, 
    BLUE_RED, 
    SEISMIC
  };

  enum ColorMapType {
    ALPHA, 
    LUM_ALPHA, 
    RGBA
  };

  // Fields
  SoSFInt32 shift;
  SoSFInt32 offset;
  SoSFEnum predefColorMap;
  SoSFEnum colorMapType;
  SoMFFloat colorMap;

  // Functions
  SoTransferFunction();
  ~SoTransferFunction();
  void reMap(int min, int max);

private:
  friend class SoTransferFunctionP;
  class SoTransferFunctionP * pimpl;

};//SoTransferFunction

#endif // !COIN_SORIO_H
