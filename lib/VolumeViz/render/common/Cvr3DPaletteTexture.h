#ifndef COIN_CVR3DPALETTETEXTURE_H
#define COIN_CVR3DPALETTETEXTURE_H

#include <Inventor/SbVec3s.h>
#include <VolumeViz/render/common/CvrPaletteTexture.h>

class CvrCLUT;


class Cvr3DPaletteTexture : public CvrPaletteTexture {
public:
  static void initClass(void);

  Cvr3DPaletteTexture(const SbVec3s & size);
  virtual ~Cvr3DPaletteTexture();

  virtual SoType getTypeId(void) const;
  static SoType getClassTypeId(void);

  SbVec3s dimensions;
  virtual uint8_t * getIndex8Buffer(void) const;

  void blankUnused(const SbVec3s & texsize) const;

private:
  static SoType classTypeId;

};

#endif // !COIN_CVR3DPALETTETEXTURE_H
