#ifndef COIN_CVRPALETTETEXTURE_H
#define COIN_CVRPALETTETEXTURE_H

#include <VolumeViz/render/2D/CvrTextureObject.h>

class CvrCLUT;


class CvrPaletteTexture : public CvrTextureObject {
public:
  static void initClass(void);

  CvrPaletteTexture(const SbVec2s & size);
  virtual ~CvrPaletteTexture();

  virtual SoType getTypeId(void) const;
  static SoType getClassTypeId(void);

  uint8_t * getIndex8Buffer(void) const;
  void setCLUT(const CvrCLUT * table);
  const CvrCLUT * getCLUT(void) const;

  virtual void dumpToPPM(const char * filename) const;

private:
  uint8_t * indexbuffer;
  const CvrCLUT * clut;
  static SoType classTypeId;
};

#endif // !COIN_CVRPALETTETEXTURE_H
