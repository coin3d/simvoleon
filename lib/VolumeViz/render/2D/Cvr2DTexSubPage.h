#ifndef COIN_CVR2DTEXSUBPAGE_H
#define COIN_CVR2DTEXSUBPAGE_H

#include <Inventor/SbVec2s.h>
#include <Inventor/misc/SoState.h>


class Cvr2DTexSubPage {
public:
  Cvr2DTexSubPage(void);
  ~Cvr2DTexSubPage();

  void setActivePage(long tick);
  void setData(unsigned char * bytes,
               const SbVec2s & size,
               const float * palette = NULL,
               int paletteFormat = 0,
               int paletteSize = 0);

  void release(void);

  // FIXME: must be public, since used from SoVolumeData. 20021106 mortene.
  unsigned long lastuse;

  // FIXME: must be public, since used from Cvr2DTexPage. 20021106 mortene.
  int numBytesHW;
  uint32_t transferFunctionId;

private:
  int format, paletteFormat, paletteSize; 
  unsigned char * palette;
  unsigned char * data;
  unsigned int textureName;
  SbVec2s size;
};


#endif //COIN_CVR2DTEXSUBPAGE_H
