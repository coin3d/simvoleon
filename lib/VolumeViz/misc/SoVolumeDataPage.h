#ifndef COIN_SOVOLUMEDATAPAGE_H
#define COIN_SOVOLUMEDATAPAGE_H

#include <Inventor/SbVec2s.h>
#include <Inventor/misc/SoState.h>

class SoVolumeDataPage{
public:
  SoVolumeDataPage();
  ~SoVolumeDataPage();

  enum Storage {
    NOT_LOADED = 0x0,
    MEMORY = 0x1,
    OPENGL = 0x2, 
  };

  void setActivePage(long tick);
  void setData( Storage storage,
                unsigned char * bytes,
                const SbVec2s & size,
                const int numcomponents,
                const int format,
                const float quality = 0.5f,
                const int border = 0);


  Storage storage; 
  int format; 
  unsigned char * data;
  unsigned int textureName;
  unsigned long lastuse;
  SbVec2s size;
  uint32_t transferFunctionId;


  SoVolumeDataPage * nextPage;
};


#endif //COIN_SOVOLUMEDATAPAGE_H