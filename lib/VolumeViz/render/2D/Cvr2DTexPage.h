#ifndef COIN_CVR2DTEXPAGE_H
#define COIN_CVR2DTEXPAGE_H

#include <Inventor/SbVec2s.h>
#include <Inventor/misc/SoState.h>
#include <VolumeViz/render/2D/Cvr2DTexSubPage.h>
#include <VolumeViz/nodes/SoOrthoSlice.h>
#include <VolumeViz/nodes/SoTransferFunction.h>
#include <VolumeViz/readers/SoVolumeReader.h>


class Cvr2DTexPage {

public:
  Cvr2DTexPage(void);
  ~Cvr2DTexPage();

  Cvr2DTexSubPage * getPage(int col, int row,
                             SoTransferFunction * transferFunction);

  Cvr2DTexSubPage * buildPage(int col, int row,
                               SoTransferFunction * transferFunction);

  void init(SoVolumeReader * reader, int sliceIdx,
            SoOrthoSlice::Axis axis, const SbVec2s & pageSize);

  void render(SoState * state,
              const SbVec3f & v0, const SbVec3f & v1,
              const SbVec3f & v2, const SbVec3f & v3,
              const SbBox2f & textureCoords,
              SoTransferFunction * transferFunction, long tick);

  Cvr2DTexSubPage * getLRUPage(void);
  void releasePage(Cvr2DTexSubPage *page);

  // FIXME: must be public, since they are used from
  // SoVolumeData. 20021106 mortene.
  int numTexels;
  int numPages;
  int numBytesHW;

private:
  void releaseLRUPage(void);
  void releaseAllPages(void);

  int calcPageIdx(int row, int col) const;

  class Cvr2DTexSubPageItem ** pages;
  SoVolumeReader * reader;

  SoOrthoSlice::Axis axis;
  int sliceIdx;
  SbVec2s pageSize;
  SbVec2s dimensions;

  int numCols;
  int numRows;

  SoVolumeData::DataType dataType;
};

#endif // !COIN_CVR2DTEXPAGE_H
