#ifndef COIN_CVRUTIL_H
#define COIN_CVRUTIL_H

#include <VolumeViz/nodes/SoVolumeData.h>


class CvrUtil {
public:
  static void buildSubPage(const unsigned int axisidx,
                           const uint8_t * input, uint8_t * output,
                           const int pageidx, const SbBox2s & cutslice,
                           const unsigned short destwidth,
                           const SoVolumeData::DataType type,
                           const SbVec3s & dim);

private:
  static void buildSubPageX(const uint8_t * input, uint8_t * output,
                            const int pageidx, const SbBox2s & cutslice,
                            const unsigned short destwidth,
                            const SoVolumeData::DataType type,
                            const SbVec3s & dim);
  static void buildSubPageY(const uint8_t * input, uint8_t * output,
                            const int pageidx, const SbBox2s & cutslice,
                            const unsigned short destwidth,
                            const SoVolumeData::DataType type,
                            const SbVec3s & dim);
  static void buildSubPageZ(const uint8_t * input, uint8_t * output,
                            const int pageidx, const SbBox2s & cutslice,
                            const unsigned short destwidth,
                            const SoVolumeData::DataType type,
                            const SbVec3s & dim);
};

#endif // !COIN_CVRUTIL_H
