#ifndef COIN_CVRRGBATEXTURE_H
#define COIN_CVRRGBATEXTURE_H

#include <VolumeViz/render/2D/CvrTextureObject.h>


class CvrRGBATexture : public CvrTextureObject {
public:
  static void initClass(void);

  CvrRGBATexture(const SbVec2s & size);
  virtual ~CvrRGBATexture();

  virtual SoType getTypeId(void) const;
  static SoType getClassTypeId(void);

  uint32_t * getRGBABuffer(void) const;
  void blankUnused(const SbVec2s & texsize);
  virtual void dumpToPPM(const char * filename) const;

private:
  uint32_t * rgbabuffer;
  static SoType classTypeId;
};

#endif // !COIN_CVRRGBATEXTURE_H
