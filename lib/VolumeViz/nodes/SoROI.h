#ifndef COIN_SOROI_H
#define COIN_SOROI_H

#include <VolumeViz/nodes/SoVolumeRendering.h>
#include <Inventor/fields/SoSFBox3s.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFEnum.h>

class SoROI : public SoVolumeRendering {
  typedef SoVolumeRendering inherited;

  SO_NODE_HEADER(SoROI);

public:
  SoROI(void);
  static void initClass(void);

  enum Flags {
    ENABLE_X0 = 0x1, 
    ENABLE_Y0 = 0x2, 
    ENABLE_Z0 = 0x4, 
    INVERT_0 = 0x8, 
    ENABLE_X1 = 0x10, 
    ENABLE_Y1 = 0x20, 
    ENABLE_Z1 = 0x40, 
    INVERT_1 = 0x80, 
    ENABLE_X2 = 0x100, 
    ENABLE_Y2 = 0x200, 
    ENABLE_Z2 = 0x400, 
    INVERT_2 = 0x800, 
    OR_SELECT = 0x1000, 
    INVERT_OUTPUT = 0x2000, 
    SUB_VOLUME = ENABLE_X0 | ENABLE_Y0 | ENABLE_Z0, 
    EXCLUSION_BOX = SUB_VOLUME | INVERT_OUTPUT, 
    CROSS = ENABLE_X0 | ENABLE_Y0 | ENABLE_Y1 | 
            ENABLE_Z1 | ENABLE_X2 | ENABLE_Z2 | OR_SELECT, 
    CROSS_INVERT = CROSS | INVERT_OUTPUT, 
    FENCE = ENABLE_X0 | ENABLE_Y1 | ENABLE_Z2 | OR_SELECT, 
    FENCE_INVERT = FENCE | INVERT_OUTPUT 
  };

  SoSFBox3s box;
  SoSFEnum flags;
  SoSFBox3s subVolume;
  SoSFBool relative;

protected:
  ~SoROI();

  virtual void GLRender(SoGLRenderAction * action);

  // FIXME: Implement these functions... torbjorv 07312002
  virtual void doAction(SoAction * action);
  virtual void callback(SoCallbackAction * action);
  virtual void getBoundingBox(SoGetBoundingBoxAction * action);
  virtual void pick(SoPickAction * action);

private:
  friend class SoROIP;
  class SoROIP * pimpl;
};

#endif // !COIN_SOROI_H
