#ifndef COIN_SOTRANSFERFUNCTION_H
#define COIN_SOTRANSFERFUNCTION_H

#include <VolumeViz/nodes/SoVolumeRendering.h>
#include <Inventor/fields/SoSFEnum.h>
#include <Inventor/fields/SoSFInt32.h>
#include <Inventor/fields/SoMFFloat.h>

class SbVec2s;


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

  SoSFInt32 shift;
  SoSFInt32 offset;
  SoSFEnum predefColorMap;
  SoSFEnum colorMapType;
  SoMFFloat colorMap;

  SoTransferFunction();

  // FIXME: Implement this function. torbjorv 08282002
  void reMap(int min, int max);



protected:
  ~SoTransferFunction();
  void GLRender(SoGLRenderAction * action);

private:
  friend class SoTransferFunctionP;
  friend class SoVolumeDataSlice;
  class SoTransferFunctionP * pimpl;

  // FIXME: move to pimpl class. 20021106 mortene.
  void transfer(const void * input, 
                SoVolumeRendering::DataType inputDataType,
                SbVec2s & size,
                void *& output,
                int &outputFormat,
                float *& palette,
                int &paletteFormat,
                int &paletteSize);

};

#endif // !COIN_SOTRANSFERFUNCTION_H
