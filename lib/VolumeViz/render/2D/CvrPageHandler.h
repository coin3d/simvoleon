#ifndef COIN_CVRPAGEHANDLER_H
#define COIN_CVRPAGEHANDLER_H

#include <Inventor/SbVec3s.h>
#include <Inventor/SbBox2f.h>

class Cvr2DTexPage;
class SoVolumeReader;
class SoState;


class CvrPageHandler {
public:
  CvrPageHandler(const SbVec3s & voldatadims, SoVolumeReader * reader);
  ~CvrPageHandler();

  Cvr2DTexPage * getSlice(const unsigned int AXISIDX, unsigned int sliceidx);

  void renderOrthoSlice(SoState * state, const SbBox2f & quad,
                        float depth, int sliceIdx, unsigned int axis);

  void releaseAllSlices(void);
  void releaseSlices(const unsigned int AXISIDX);

private:
  Cvr2DTexPage ** slices[3];
  unsigned int voldatadims[3];
  unsigned int subpagesize[3];
  SoVolumeReader * reader;
};

#endif // !COIN_CVRPAGEHANDLER_H
