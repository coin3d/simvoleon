#ifndef COIN_SOVOLUMEDATASLICE_H
#define COIN_SOVOLUMEDATASLICE_H

#include <Inventor/SbVec2s.h>
#include <Inventor/misc/SoState.h>
#include <VolumeViz/misc/SoVolumeDataPage.h>
#include <VolumeViz/nodes/SoTransferFunction.h>

class SoVolumeDataSlice{
public:
  SoVolumeDataSlice();
  ~SoVolumeDataSlice();

  void setActivePage( int col, 
                      int row, 
                      SoTransferFunction *transferFunction, 
                      int tick);

  void releaseOldestPage();
  void releaseAllPages();

  SoVolumeDataPage * pages;
  SbVec2s size;
};


#endif //COIN_SOVOLUMEDATASLICE_H