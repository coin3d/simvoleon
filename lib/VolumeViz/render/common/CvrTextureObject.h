#ifndef COIN_CVRTEXTUREOBJECT_H
#define COIN_CVRTEXTUREOBJECT_H

#include <Inventor/SbVec2s.h>
#include <Inventor/SoType.h>


class CvrTextureObject {
public:
  static void initClass(void);

  CvrTextureObject();
  virtual ~CvrTextureObject();

  virtual SoType getTypeId(void) const = 0;
  static SoType getClassTypeId(void);

private:
  static SoType classTypeId;

};

#endif // !COIN_CVRTEXTUREOBJECT_H
