#ifndef COIN_CVRVOXELCHUNK_H
#define COIN_CVRVOXELCHUNK_H

#include <Inventor/SbVec3s.h>
#include <VolumeViz/nodes/SoTransferFunction.h>

class CvrTextureObject;
class SoGLRenderAction;


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

  CvrTextureObject * transfer(SoGLRenderAction * action, SbBool & invisible) const;

private:
  SbBool destructbuffer;
  void * voxelbuffer;
  SbVec3s dimensions;
  UnitSize unitsize;

  // The simple idea for speeding up transfer of volume data is to
  // dynamically fill in an index array, so each transfer value
  // calculation is done only once.
  static void blankoutTransferTable(void);
  static uint32_t transfertable[256];
  static SbBool transferdone[256];
  static uint32_t transfertablenodeid;

  static uint8_t PREDEFGRADIENTS[SoTransferFunction::SEISMIC + 1][256][4];
  static void initPredefGradients(void);
};

#endif // !COIN_CVRVOXELCHUNK_H
