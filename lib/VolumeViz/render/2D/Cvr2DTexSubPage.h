#ifndef COIN_CVR2DTEXSUBPAGE_H
#define COIN_CVR2DTEXSUBPAGE_H

#include <Inventor/SbVec2s.h>
#include <Inventor/SbVec2f.h>
#include <Inventor/SbVec3f.h>
#include <Inventor/misc/SoState.h>
#include <Inventor/system/gl.h>

class SoGLRenderAction;
class CvrTextureObject;


class Cvr2DTexSubPage {
public:
  Cvr2DTexSubPage(SoGLRenderAction * action,
                  const CvrTextureObject * texobj,
                  const SbVec2s & pagesize, const SbVec2s & texsize);
  ~Cvr2DTexSubPage();

  enum Interpolation { NEAREST, LINEAR };

  void render(const SbVec3f & upleft, SbVec3f widthvec, SbVec3f heightvec,
              Interpolation interpolation) const;

  static unsigned int totalNrOfTexels(void);
  static unsigned int totalTextureMemoryUsed(void);

private:
  void transferTex2GL(SoGLRenderAction * action,
                      const CvrTextureObject * texobj);

  void activateTexture(Interpolation interpolation) const;

  GLuint texturename[1];
  static GLuint emptyimgname[1];
  SbVec2s texdims;
  SbVec2f texmaxcoords;
  SbVec2f quadpartfactors;
  static unsigned int nroftexels;
  static unsigned int texmembytes;
  static SbBool detectedtextureswapping;
  unsigned int bitspertexel;
};


#endif //COIN_CVR2DTEXSUBPAGE_H
