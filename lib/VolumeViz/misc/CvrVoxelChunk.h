#ifndef COIN_CVRVOXELCHUNK_H
#define COIN_CVRVOXELCHUNK_H

#include <Inventor/SbVec3s.h>
#include <Inventor/SbBox3s.h>
#include <VolumeViz/nodes/SoTransferFunction.h>

class CvrTextureObject;
class SoGLRenderAction;
class CvrCLUT;
class SoTransferFunctionElement;
class SbBox2s;


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

  CvrTextureObject * transfer2D(SoGLRenderAction * action, SbBool & invisible) const;
  CvrTextureObject * transfer3D(SoGLRenderAction * action, SbBool & invisible) const;
  
  void dumpToPPM(const char * filename) const;

  // FIXME: move to CvrCLUT?
  static CvrCLUT * getCLUT(const SoTransferFunctionElement * e);

  CvrVoxelChunk * buildSubPage(const unsigned int axisidx, const int pageidx,
                               const SbBox2s & cutslice);

  CvrVoxelChunk * buildSubCube(const SbBox3s & cubecut);

private:
  CvrVoxelChunk * buildSubPageX(const int pageidx, const SbBox2s & cutslice);
  CvrVoxelChunk * buildSubPageY(const int pageidx, const SbBox2s & cutslice);
  CvrVoxelChunk * buildSubPageZ(const int pageidx, const SbBox2s & cutslice);

  SbBool destructbuffer;
  void * voxelbuffer;
  SbVec3s dimensions;
  UnitSize unitsize;

  static CvrCLUT * makeCLUT(const SoTransferFunctionElement * e);
  static SbDict * CLUTdict;

  static SbBool usePaletteTextures(SoGLRenderAction * action);

  static uint8_t PREDEFGRADIENTS[SoTransferFunction::SEISMIC + 1][256][4];
  static void initPredefGradients(void);
};

#endif // !COIN_CVRVOXELCHUNK_H
