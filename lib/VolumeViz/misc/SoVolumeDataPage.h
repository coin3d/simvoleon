#ifndef COIN_SOVOLUMEDATAPAGE_H
#define COIN_SOVOLUMEDATAPAGE_H

#include <Inventor/SbVec2s.h>
#include <Inventor/misc/SoState.h>


class SoVolumeDataPage {
public:
  SoVolumeDataPage(void);
  ~SoVolumeDataPage();

  enum Storage {
    NOT_LOADED = 0x0,
    MEMORY = 0x1,
    OPENGL = 0x2
  };

  void setActivePage(long tick);
  void setData(Storage storage,
               unsigned char * bytes,
               const SbVec2s & size,
               const float * palette = NULL,
               int paletteFormat = 0,
               int paletteSize = 0);

  void release(void);

  // FIXME: must be public, since they are used from
  // SoVolumeData. 20021106 mortene.
  unsigned long lastuse;

  // FIXME: must be public, since they are used from
  // SoVolumeDataSlice. 20021106 mortene.
  int numBytesHW;
  uint32_t transferFunctionId;

private:
  Storage storage; 
  int format, paletteFormat, paletteSize; 
  unsigned char * palette;
  unsigned char * data;
  unsigned int textureName;
  SbVec2s size;
};


#endif //COIN_SOVOLUMEDATAPAGE_H
