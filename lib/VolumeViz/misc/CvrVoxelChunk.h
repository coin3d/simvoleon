#ifndef COIN_CVRVOXELCHUNK_H
#define COIN_CVRVOXELCHUNK_H

#include <Inventor/SbVec3s.h>


class CvrVoxelChunk {
public:
  // Note that enum values matches nr of bytes. Don't change this.
  enum UnitSize { UINT_8 = 1, UINT_16 = 2, UINT_32 = 4 };

  CvrVoxelChunk(const SbVec3s & dimensions, UnitSize type,
                void * buffer = NULL);
  ~CvrVoxelChunk();

  void * getBuffer(void) const;
  uint8_t * getBuffer8(void) const;
  uint16_t * getBuffer16(void) const;
  uint32_t * getBuffer32(void) const;

  unsigned int bufferSize(void) const;

  const SbVec3s & getDimensions(void) const;
  UnitSize getUnitSize(void) const;

private:
  SbBool destructbuffer;
  void * voxelbuffer;
  SbVec3s dimensions;
  UnitSize unitsize;
};

#endif // !COIN_CVRVOXELCHUNK_H
