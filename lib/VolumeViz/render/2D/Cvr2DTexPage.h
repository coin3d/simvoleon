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

  void render(SoGLRenderAction * action, const SbVec3f & origo,
              const SbVec3f & horizspan, const SbVec3f & verticalspan,
              const SbVec2f & spacescale,
              Cvr2DTexSubPage::Interpolation interpolation);

private:
  class Cvr2DTexSubPageItem * getSubPage(SoState * state, int col, int row);

  class Cvr2DTexSubPageItem * buildSubPage(SoGLRenderAction * action,
                                           int col, int row);

  void releaseSubPage(Cvr2DTexSubPage * page);

  void releaseAllSubPages(void);
  void releaseSubPage(const int row, const int col);

  int calcSubPageIdx(int row, int col) const;

  static SoTransferFunction * getTransferFunc(SoGLRenderAction * action);

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
