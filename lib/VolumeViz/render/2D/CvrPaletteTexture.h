#ifndef COIN_CVRPALETTETEXTURE_H
#define COIN_CVRPALETTETEXTURE_H

#include <VolumeViz/render/2D/CvrTextureObject.h>


class CvrPaletteTexture : public CvrTextureObject {
public:
  static void initClass(void);

  CvrPaletteTexture(const SbVec2s & size);
  virtual ~CvrPaletteTexture();

  virtual SoType getTypeId(void) const;
  static SoType getClassTypeId(void);

private:
  static SoType classTypeId;
};

#endif // !COIN_CVRPALETTETEXTURE_H
