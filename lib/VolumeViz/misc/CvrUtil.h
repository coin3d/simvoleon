#ifndef COIN_CVRUTIL_H
#define COIN_CVRUTIL_H

#include <VolumeViz/nodes/SoVolumeData.h>


class CvrUtil {
public:
  static void buildSubSliceX(const void * input, void * output,
                             const int pageidx, const SbBox2s & cutslice,
                             const unsigned short destwidth,
                             const SoVolumeData::DataType type, const SbVec3s & dim);
  static void buildSubSliceY(const void * input, void * output,
                             const int pageidx, const SbBox2s & cutslice,
                             const unsigned short destwidth,
                             const SoVolumeData::DataType type, const SbVec3s & dim);
  static void buildSubSliceZ(const void * input, void * output,
                             const int pageidx, const SbBox2s & cutslice,
                             const unsigned short destwidth,
                             const SoVolumeData::DataType type, const SbVec3s & dim);
};

#endif // !COIN_CVRUTIL_H
