#ifndef COIN_CVRPAGEHANDLER_H
#define COIN_CVRPAGEHANDLER_H

#include <Inventor/SbBox2f.h>
#include <Inventor/SbVec3f.h>
#include <Inventor/SbVec3s.h>

class Cvr2DTexPage;
class SoVolumeReader;
class SoState;
class SoGLRenderAction;


class CvrPageHandler {
public:
  CvrPageHandler(const SbVec3s & voldatadims, SoVolumeReader * reader);
  ~CvrPageHandler();

  void render(SoGLRenderAction * action, int numslices);
  unsigned int getCurrentAxis(SoGLRenderAction * action) const;

  void releaseAllSlices(void);
  void releaseSlices(const unsigned int AXISIDX);

private:
  unsigned int getCurrentAxis(const SbVec3f & viewvec) const;
  void getViewVector(SoGLRenderAction * action, SbVec3f & direction) const;

  Cvr2DTexPage * getSlice(const unsigned int AXISIDX, unsigned int sliceidx);

  void renderOrthoSlice(SoGLRenderAction * action, const SbBox2f & quad,
                        float depth, int sliceIdx, unsigned int axis);

  Cvr2DTexPage ** slices[3];
  unsigned int voldatadims[3];
  unsigned int subpagesize[3];
  SoVolumeReader * reader;
};

#endif // !COIN_CVRPAGEHANDLER_H
