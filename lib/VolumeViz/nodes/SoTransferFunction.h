#ifndef COIN_SOTRANSFERFUNCTION_H
#define COIN_SOTRANSFERFUNCTION_H

#include <VolumeViz/nodes/SoVolumeRendering.h>
#include <Inventor/fields/SoSFEnum.h>
#include <Inventor/fields/SoSFInt32.h>
#include <Inventor/fields/SoMFFloat.h>

// FIXME: get rid of this when private method is actually made
// private. 20021107 mortene.
#include <VolumeViz/nodes/SoVolumeData.h>

class SbVec2s;


class SoTransferFunction : public SoVolumeRendering {
  typedef SoVolumeRendering inherited;

  SO_NODE_HEADER(SoTransferFunction);

public:
  static void initClass(void);
  SoTransferFunction(void);

  enum PredefColorMap {
    NONE = 0,
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

  SoSFInt32 shift;
  SoSFInt32 offset;
  SoSFEnum predefColorMap;
  SoSFEnum colorMapType;
  SoMFFloat colorMap;

  void reMap(int low, int high);


protected:
  ~SoTransferFunction();

  virtual void doAction(SoAction * action);
  virtual void GLRender(SoGLRenderAction * action);
  virtual void callback(SoCallbackAction * action);
  virtual void pick(SoPickAction * action);

private:
  friend class SoTransferFunctionP;
  class SoTransferFunctionP * pimpl;

  // FIXME: clean up interface. 20021106 mortene.
  friend class Cvr2DTexPage;
  uint32_t * transfer(const uint8_t * input,
                      SoVolumeData::DataType inputdatatype,
                      const SbVec2s & size, SbBool & invisible) const;
};

#endif // !COIN_SOTRANSFERFUNCTION_H
