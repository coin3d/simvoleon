#ifndef COIN_CVRPAGEHANDLER_H
#define COIN_CVRPAGEHANDLER_H

#include <Inventor/SbBox2f.h>
#include <Inventor/SbVec3f.h>
#include <Inventor/SbVec3s.h>
#include <VolumeViz/render/2D/Cvr2DTexSubPage.h>
#include <VolumeViz/nodes/SoVolumeRender.h>

class Cvr2DTexPage;
class SoVolumeReader;
class SoState;
class SoGLRenderAction;


class CvrPageHandler {
public:
  CvrPageHandler(const SbVec3s & voldatadims, SoVolumeReader * reader);
  ~CvrPageHandler();

  enum Composition { MAX_INTENSITY, SUM_INTENSITY, ALPHA_BLENDING };

  void render(SoGLRenderAction * action, unsigned int numslices,
              Cvr2DTexSubPage::Interpolation interpolation,
              CvrPageHandler::Composition composition,
              SoVolumeRender::SoVolumeRenderAbortCB * abortfunc,
              void * abortcbdata);

  unsigned int getCurrentAxis(SoGLRenderAction * action) const;

  void releaseAllSlices(void);
  void releaseSlices(const unsigned int AXISIDX);

private:
  unsigned int getCurrentAxis(const SbVec3f & viewvec) const;
  void getViewVector(SoGLRenderAction * action, SbVec3f & direction) const;
  Cvr2DTexPage * getSlice(const unsigned int AXISIDX, unsigned int sliceidx);
  void comparePageSize(const SbVec3s & currsubpagesize);

  Cvr2DTexPage ** slices[3];
  unsigned int voldatadims[3];
  SbVec3s subpagesize;
  SoVolumeReader * reader;

  uint32_t transferfuncid;
};

#endif // !COIN_CVRPAGEHANDLER_H
