#ifndef COIN_CVRUTIL_H
#define COIN_CVRUTIL_H

#include <VolumeViz/nodes/SoVolumeData.h>

class CvrVoxelChunk;


class CvrUtil {
public:
  static CvrVoxelChunk * buildSubPage(const CvrVoxelChunk & input,
                                      const unsigned int axisidx, const int pageidx,
                                      const SbBox2s & cutslice);

private:
  static CvrVoxelChunk * buildSubPageX(const CvrVoxelChunk & input,
                                       const int pageidx,
                                       const SbBox2s & cutslice);

  static CvrVoxelChunk * buildSubPageY(const CvrVoxelChunk & input,
                                       const int pageidx,
                                       const SbBox2s & cutslice);

  static CvrVoxelChunk * buildSubPageZ(const CvrVoxelChunk & input,
                                       const int pageidx,
                                       const SbBox2s & cutslice);
};

#endif // !COIN_CVRUTIL_H
