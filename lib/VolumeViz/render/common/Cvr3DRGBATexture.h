#ifndef COIN_CVR3DRGBATEXTURE_H
#define COIN_CVR3DRGBATEXTURE_H

#include <Inventor/SbVec3s.h>
#include <VolumeViz/render/common/CvrRGBATexture.h>

class Cvr3DRGBATexture : public CvrRGBATexture {
public:
  static void initClass(void);

  Cvr3DRGBATexture(const SbVec3s & size);
  virtual ~Cvr3DRGBATexture();

  virtual SoType getTypeId(void) const;
  static SoType getClassTypeId(void);

  SbVec3s dimensions;

  virtual uint32_t * getRGBABuffer(void) const;
  void blankUnused(const SbVec3s & texsize) const;

private:
  static SoType classTypeId;

};

#endif // !COIN_CVR3DRGBATEXTURE_H
