#ifndef COIN_CVR2DTEXSUBPAGE_H
#define COIN_CVR2DTEXSUBPAGE_H

#include <Inventor/SbVec2s.h>
#include <Inventor/misc/SoState.h>
#include <Inventor/system/gl.h>


class Cvr2DTexSubPage {
public:
  Cvr2DTexSubPage(const uint8_t * textureimg, const SbVec2s & size,
                  const float * palette = NULL, int palettesize = 0);
  ~Cvr2DTexSubPage();

  void activate(void) const;

  static unsigned int totalNrOfTexels(void);
  static unsigned int totalTextureMemoryUsed(void);

private:
  void transferTex2GL(const uint8_t * textureimg,
                      int palettesize, const float * palette);

  GLuint texturename[1];
  static GLuint emptyimgname[1];
  SbVec2s texdims;
  static unsigned int nroftexels;
  static unsigned int texmembytes;
  float texmultfactor;
};


#endif //COIN_CVR2DTEXSUBPAGE_H
