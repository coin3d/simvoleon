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

  void render(SoGLRenderAction * action, unsigned int numslices);
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
};

#endif // !COIN_CVRPAGEHANDLER_H
