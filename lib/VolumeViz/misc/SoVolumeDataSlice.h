#ifndef COIN_SOVOLUMEDATASLICE_H
#define COIN_SOVOLUMEDATASLICE_H

#include <Inventor/SbVec2s.h>
#include <Inventor/misc/SoState.h>
#include <VolumeViz/misc/SoVolumeDataPage.h>
#include <VolumeViz/nodes/SoTransferFunction.h>
#include <VolumeViz/readers/SoVolumeReader.h>

class SoVolumeDataSlice {

public:
  SoVolumeDataSlice(void);
  ~SoVolumeDataSlice();

  SoVolumeDataPage * getPage(int col, int row,
                             SoTransferFunction * transferFunction);

  SoVolumeDataPage * buildPage(int col, int row,
                               SoTransferFunction * transferFunction);

  void init(SoVolumeReader * reader, int sliceIdx,
            SoOrthoSlice::Axis axis, const SbVec2s & pageSize);

  void render(SoState * state,
              const SbVec3f & v0, const SbVec3f & v1,
              const SbVec3f & v2, const SbVec3f & v3,
              const SbBox2f & textureCoords,
              SoTransferFunction * transferFunction, long tick);

  SoVolumeDataPage * getLRUPage(void);
  void releasePage(SoVolumeDataPage *page);

  // FIXME: must be public, since they are used from
  // SoVolumeData. 20021106 mortene.
  int numTexels;
  int numPages;
  int numBytesHW;
  int numBytesSW;

private:
  void releaseLRUPage(void);
  void releaseAllPages(void);

  SoVolumeDataPage ** pages;
  SoVolumeReader * reader;

  SoOrthoSlice::Axis axis;
  int sliceIdx;
  SbVec2s pageSize;
  SbVec2s dimensions;

  int numCols;
  int numRows;

  SoVolumeData::DataType dataType;
};

#endif // !COIN_SOVOLUMEDATASLICE_H
