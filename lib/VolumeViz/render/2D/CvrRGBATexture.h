#ifndef COIN_CVRRGBATEXTURE_H
#define COIN_CVRRGBATEXTURE_H

#include <VolumeViz/render/2D/CvrTextureObject.h>


class CvrRGBATexture : public CvrTextureObject {
public:
  static void initClass(void);

  CvrRGBATexture(const SbVec2s & size);
  virtual ~CvrRGBATexture();

  uint32_t * getRGBABuffer(void) const;

  virtual SoType getTypeId(void) const;
  static SoType getClassTypeId(void);

private:
  uint32_t * rgbabuffer;
  static SoType classTypeId;
};

#endif // !COIN_CVRRGBATEXTURE_H
