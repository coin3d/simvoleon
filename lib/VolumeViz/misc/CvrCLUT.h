#ifndef COIN_CVRCLUT_H
#define COIN_CVRCLUT_H

#include <Inventor/system/inttypes.h>
#include <Inventor/C/glue/gl.h>

class CvrCLUT {
public:
  CvrCLUT(const unsigned int nrcols, const uint8_t * colormap);
  CvrCLUT(const unsigned int nrcols, const unsigned int nrcomponents,
          const float * colormap);
  CvrCLUT(const CvrCLUT & clut);

  friend int operator==(const CvrCLUT & c1, const CvrCLUT & c2);
  friend int operator!=(const CvrCLUT & c1, const CvrCLUT & c2);

  void ref(void) const;
  void unref(void) const;
  int32_t getRefCount(void) const;

  void setTransparencyThresholds(uint32_t low, uint32_t high);

  void activate(const cc_glglue * glw) const;
  void deactivate(const cc_glglue * glw) const;
  void lookupRGBA(const unsigned int idx, uint8_t rgba[4]) const;

  // Note: must match the enum in SoOrthoSlice.
  enum AlphaUse { ALPHA_AS_IS = 0, ALPHA_OPAQUE = 1, ALPHA_BINARY = 2 };
  enum TextureType { TEXTURE2D, TEXTURE3D };

  void setAlphaUse(AlphaUse policy);
  void setTextureType(TextureType type);

private:
  ~CvrCLUT();
  void commonConstructor(void);
  void regenerateGLColorData(void);
  void initFragmentProgram(const cc_glglue * glue);
  void initPaletteTexture(const cc_glglue * glue);

  unsigned int nrentries;
  unsigned int nrcomponents;


  enum DataType { INTS, FLOATS } datatype;
  union {
    uint8_t * int_entries;
    float * flt_entries;
  };
  uint32_t crc32cmap;

  uint32_t transparencythresholds[2];
  AlphaUse alphapolicy;

  uint8_t * glcolors;

  SbBool palettehaschanged;
  SbBool usefragmentprogramlookup;
  SbBool fragmentprograminitialized;
  GLuint palettelookuptexture;
  GLuint palettelookupprogramid;
  TextureType texturetype;

  int refcount;
  friend class nop; // to avoid g++ compiler warning on the private constructor
};

int operator==(const CvrCLUT & c1, const CvrCLUT & c2);
int operator!=(const CvrCLUT & c1, const CvrCLUT & c2);

#endif // !COIN_CVRCLUT_H
