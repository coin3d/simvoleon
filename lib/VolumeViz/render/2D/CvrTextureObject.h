#ifndef COIN_CVRTEXTUREOBJECT_H
#define COIN_CVRTEXTUREOBJECT_H

#include <Inventor/SbVec2s.h>
#include <Inventor/SoType.h>


class CvrTextureObject {
public:
  static void initClass(void);

  CvrTextureObject(const SbVec2s & size);
  virtual ~CvrTextureObject();

  const SbVec2s & getDimensions(void) const;

  virtual SoType getTypeId(void) const = 0;
  static SoType getClassTypeId(void);

private:
  SbVec2s dimensions;
  static SoType classTypeId;
};

#endif // !COIN_CVRTEXTUREOBJECT_H
