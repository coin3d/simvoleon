#ifndef COIN_CVRRGBATEXTURE_H
#define COIN_CVRRGBATEXTURE_H

#include <VolumeViz/render/common/CvrTextureObject.h>

class CvrRGBATexture : public CvrTextureObject {
public:
  static void initClass(void);

  CvrRGBATexture();
  virtual ~CvrRGBATexture();

  virtual SoType getTypeId(void) const;
  static SoType getClassTypeId(void);

  virtual uint32_t * getRGBABuffer(void) const;

private:
  friend class Cvr2DRGBATexture;
  friend class Cvr3DRGBATexture; 

  uint32_t * rgbabuffer;
  static SoType classTypeId;

};

#endif // !COIN_CVRRGBATEXTURE_H
