#ifndef COIN_CVR2DTEXPAGE_H
#define COIN_CVR2DTEXPAGE_H

#include <Inventor/SbVec2s.h>
#include <Inventor/misc/SoState.h>
#include <VolumeViz/render/2D/Cvr2DTexSubPage.h>
#include <VolumeViz/readers/SoVolumeReader.h>

class SoTransferFunction;


class Cvr2DTexPage {

public:
  Cvr2DTexPage(void);
  ~Cvr2DTexPage();

  void init(SoVolumeReader * reader, int sliceIdx,
            unsigned int axis, const SbVec2s & pageSize);

  void render(SoState * state, const SbVec3f & origo,
              const SbVec3f & horizspan, const SbVec3f & verticalspan,
              long tick);

  Cvr2DTexSubPage * getLRUSubPage(long & tick);
  void releaseSubPage(Cvr2DTexSubPage *page);

private:
  class Cvr2DTexSubPageItem * getSubPage(int col, int row,
                                         SoTransferFunction * transferfunc);

  class Cvr2DTexSubPageItem * buildSubPage(int col, int row,
                                           SoTransferFunction * transferfunc);

  void releaseLRUSubPage(void);
  void releaseAllSubPages(void);

  int calcSubPageIdx(int row, int col) const;

  void renderGLQuad(const SbVec3f & lowerLeft,
                    const SbVec3f & lowerRight,
                    const SbVec3f & upperLeft,
                    const SbVec3f & upperRight);

  class Cvr2DTexSubPageItem ** subpages;
  SoVolumeReader * reader;

  unsigned int axis;
  int sliceIdx;
  SbVec2s subpagesize;
  SbVec2s dimensions;

  int nrcolumns;
  int nrrows;

  SoVolumeData::DataType dataType;
};

#endif // !COIN_CVR2DTEXPAGE_H
