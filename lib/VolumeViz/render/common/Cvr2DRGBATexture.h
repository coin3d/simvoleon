#ifndef COIN_CVR2DRGBATEXTURE_H
#define COIN_CVR2DRGBATEXTURE_H

#include <Inventor/SbVec2s.h>
#include <VolumeViz/render/common/CvrRGBATexture.h>

class Cvr2DRGBATexture : public CvrRGBATexture {
public:
  static void initClass(void);

  Cvr2DRGBATexture(const SbVec2s & size);
  virtual ~Cvr2DRGBATexture();

  virtual SoType getTypeId(void) const;
  static SoType getClassTypeId(void);

  SbVec2s dimensions;

  virtual uint32_t * getRGBABuffer(void) const;
  void blankUnused(const SbVec2s & texsize) const;

private:
  static SoType classTypeId;

};

#endif // !COIN_CVR2DRGBATEXTURE_H
