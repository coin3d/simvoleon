#ifndef COIN_CVR2DTEXSUBPAGE_H
#define COIN_CVR2DTEXSUBPAGE_H

#include <Inventor/SbVec2s.h>
#include <Inventor/SbVec2f.h>
#include <Inventor/SbVec3f.h>
#include <Inventor/misc/SoState.h>
#include <Inventor/system/gl.h>

class SoGLRenderAction;


class Cvr2DTexSubPage {
public:
  Cvr2DTexSubPage(SoGLRenderAction * action,
                  const uint8_t * textureimg,
                  const SbVec2s & pagesize, const SbVec2s & texsize,
                  const float * palette = NULL, int palettesize = 0);
  ~Cvr2DTexSubPage();

  void render(const SbVec3f & upleft, SbVec3f widthvec, SbVec3f heightvec) const;

  static unsigned int totalNrOfTexels(void);
  static unsigned int totalTextureMemoryUsed(void);

private:
  void transferTex2GL(SoGLRenderAction * action, const uint8_t * textureimg,
                      int palettesize, const float * palette);

  void activateTexture(void) const;

  GLuint texturename[1];
  static GLuint emptyimgname[1];
  SbVec2s texdims;
  SbVec2f texmaxcoords;
  static unsigned int nroftexels;
  static unsigned int texmembytes;
  static SbBool detectedtextureswapping;
  float texmultfactor;
};


#endif //COIN_CVR2DTEXSUBPAGE_H
