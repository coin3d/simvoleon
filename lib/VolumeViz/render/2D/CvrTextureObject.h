#ifndef COIN_CVRTEXTUREOBJECT_H
#define COIN_CVRTEXTUREOBJECT_H

#include <Inventor/SbVec2s.h>


class CvrTextureObject {
public:
  CvrTextureObject(const SbVec2s & size);
  ~CvrTextureObject();

  uint32_t * getRGBABuffer(void);
  const SbVec2s & getDimensions(void) const;

private:
  SbVec2s dimensions;
  uint32_t * rgbabuffer;
};

#endif // !COIN_CVRTEXTUREOBJECT_H
