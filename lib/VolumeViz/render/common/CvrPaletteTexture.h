#ifndef COIN_CVRPALETTETEXTURE_H
#define COIN_CVRPALETTETEXTURE_H

#include <VolumeViz/render/common/CvrTextureObject.h>

class CvrCLUT;


class CvrPaletteTexture : public CvrTextureObject {
public:
  static void initClass(void);

  CvrPaletteTexture();
  virtual ~CvrPaletteTexture();

  virtual SoType getTypeId(void) const;
  static SoType getClassTypeId(void);

  virtual uint8_t * getIndex8Buffer(void) const;

  void setCLUT(const CvrCLUT * table);
  const CvrCLUT * getCLUT(void) const;

private:
  friend class Cvr2DPaletteTexture;
  friend class Cvr3DPaletteTexture;

  uint8_t * indexbuffer;
  const CvrCLUT * clut;
  static SoType classTypeId;

};

#endif // !COIN_CVRPALETTETEXTURE_H
