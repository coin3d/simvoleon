#ifndef COIN_CVR2DTEXSUBPAGE_H
#define COIN_CVR2DTEXSUBPAGE_H

#include <Inventor/SbVec2s.h>
#include <Inventor/SbVec3f.h>
#include <Inventor/misc/SoState.h>
#include <Inventor/system/gl.h>

class SoGLRenderAction;


class Cvr2DTexSubPage {
public:
  Cvr2DTexSubPage(SoGLRenderAction * action,
                  const uint8_t * textureimg, const SbVec2s & size,
                  const float * palette = NULL, int palettesize = 0);
  ~Cvr2DTexSubPage();

  void render(const SbVec3f & lowerLeft, const SbVec3f & lowerRight,
              const SbVec3f & upperLeft, const SbVec3f & upperRight) const;

  static unsigned int totalNrOfTexels(void);
  static unsigned int totalTextureMemoryUsed(void);

private:
  void transferTex2GL(SoGLRenderAction * action, const uint8_t * textureimg,
                      int palettesize, const float * palette);

  void activateTexture(void) const;

  GLuint texturename[1];
  static GLuint emptyimgname[1];
  SbVec2s texdims;
  static unsigned int nroftexels;
  static unsigned int texmembytes;
  static SbBool detectedtextureswapping;
  float texmultfactor;
};


#endif //COIN_CVR2DTEXSUBPAGE_H
