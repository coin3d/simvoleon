#ifndef COIN_SOVOLUMERENDERING_H
#define COIN_SOVOLUMERENDERING_H

#include <Inventor/nodes/SoNode.h>
#include <Inventor/nodes/SoSubNode.h>

class SoVolumeRendering : public SoNode {
  typedef SoNode inherited;

  SO_NODE_HEADER(SoVolumeRendering);

public:
  static void init(void);

  static void initClass(void);
  SoVolumeRendering();

  enum HW_Feature {
    HW_VOLUMEPRO,
    HW_3DTEXMAP,
    HW_TEXCOLORMAP,
    HW_TEXCOMPRESSION
  };

  enum HW_SupportStatus { NO, YES, UNKNOWN };

  enum Axis { X, Y, Z };

  HW_SupportStatus isSupported(HW_Feature feature);

protected:
  ~SoVolumeRendering();

private:
  friend class SoVolumeRenderingP;
  class SoVolumeRenderingP * pimpl;
};

#endif // !COIN_SOVOLUMERENDERING_H
