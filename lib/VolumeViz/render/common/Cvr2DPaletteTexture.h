#ifndef COIN_CVR2DPALETTETEXTURE_H
#define COIN_CVR2DPALETTETEXTURE_H

#include <Inventor/SbVec2s.h>
#include <VolumeViz/render/common/CvrPaletteTexture.h>

class CvrCLUT;


class Cvr2DPaletteTexture : public CvrPaletteTexture {
public:
  static void initClass(void);

  Cvr2DPaletteTexture(const SbVec2s & size);
  virtual ~Cvr2DPaletteTexture();

  virtual SoType getTypeId(void) const;
  static SoType getClassTypeId(void);

  SbVec2s dimensions;
  virtual uint8_t * getIndex8Buffer(void) const;

  void blankUnused(const SbVec2s & texsize) const;
  void dumpToPPM(const char * filename) const;

private:
  static SoType classTypeId;

};

#endif // !COIN_CVR2DPALETTETEXTURE_H
