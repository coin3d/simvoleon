/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/

#ifndef COIN_SOVOLUMERENDERING_H
#define COIN_SOVOLUMERENDERING_H

#include <Inventor/nodes/SoNode.h>
#include <Inventor/nodes/SoSubNode.h>


class SoVolumeRendering : public SoNode {
  typedef SoNode inherited;

  SO_NODE_HEADER(SoVolumeRendering);

public:
  static void initClass(void);

  enum HW_Feature {
    HW_VOLUMEPRO, 
    HW_3DTEXMAP, 
    HW_TEXCOLORMAP, 
    HW_TEXCOMPRESSION
  };

  enum HW_SupportStatus {
    NO, 
    YES, 
    UNKNOWN
  };

  enum Axis {
    X, 
    Y, 
    Z
  };

  enum DataType {
    UNSIGNED_BYTE,
    UNSIGNED_SHORT,
    RGB,
    RGBA
  };

  SoVolumeRendering();
  ~SoVolumeRendering();
  static void init(); 
  HW_SupportStatus isSupported(HW_Feature feature);


private:
  friend class SoVolumeRenderingP;
  class SoVolumeRenderingP * pimpl;
};//SoVolumeRendering

#endif // !COIN_SOVOLUMERENDERING_H
