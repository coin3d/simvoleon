#ifndef COIN_CVR2DTEXPAGE_H
#define COIN_CVR2DTEXPAGE_H

#include <Inventor/SbVec2s.h>
#include <Inventor/misc/SoState.h>
#include <VolumeViz/render/2D/Cvr2DTexSubPage.h>
#include <VolumeViz/readers/SoVolumeReader.h>


class Cvr2DTexPage {

public:
  Cvr2DTexPage(SoVolumeReader * reader,
               const unsigned int axis, const unsigned int sliceidx,
               const SbVec2s & subpagetexsize);
  ~Cvr2DTexPage();

  void render(SoGLRenderAction * action, const SbVec3f & origo,
              const SbVec3f & horizspan, const SbVec3f & verticalspan,
              Cvr2DTexSubPage::Interpolation interpolation);

  void setPalette(const CvrCLUT * c);
  const CvrCLUT * getPalette(void) const;

private:
  class Cvr2DTexSubPageItem * getSubPage(SoState * state, int col, int row);

  class Cvr2DTexSubPageItem * buildSubPage(SoGLRenderAction * action,
                                           int col, int row);

  void releaseSubPage(Cvr2DTexSubPage * page);

  void releaseAllSubPages(void);
  void releaseSubPage(const int row, const int col);

  int calcSubPageIdx(int row, int col) const;

  class Cvr2DTexSubPageItem ** subpages;
  SoVolumeReader * reader;

  unsigned int axis;
  unsigned int sliceidx;
  SbVec2s subpagesize;
  SbVec2s dimensions;

  int nrcolumns;
  int nrrows;

  const CvrCLUT * clut;
};

#endif // !COIN_CVR2DTEXPAGE_H
