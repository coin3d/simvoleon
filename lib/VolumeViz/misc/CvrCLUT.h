#ifndef COIN_CVRCLUT_H
#define COIN_CVRCLUT_H

#include <Inventor/system/inttypes.h>
#include <Inventor/C/glue/gl.h>

class CvrCLUT {
public:
  CvrCLUT(const unsigned int nrcols, const uint8_t * colormap);
  CvrCLUT(const unsigned int nrcols, const unsigned int nrcomponents,
          const float * colormap);

  void ref(void) const;
  void unref(void) const;
  int32_t getRefCount(void) const;

  void setTransparencyThresholds(uint32_t low, uint32_t high);

  void activate(const cc_glglue * glw) const;
  void lookupRGBA(const unsigned int idx, uint8_t rgba[4]) const;

  // Note: must match the enum in SoOrthoSlice.
  enum AlphaUse { ALPHA_AS_IS, ALPHA_OPAQUE, ALPHA_BINARY };

  void setAlphaUse(AlphaUse policy);

private:
  ~CvrCLUT();
  void commonConstructor(void);
  void regenerateGLColorData(void);

  unsigned int nrentries;
  unsigned int nrcomponents;

  enum DataType { INTS, FLOATS } datatype;
  union {
    uint8_t * int_entries;
    float * flt_entries;
  };

  uint32_t transparencythresholds[2];
  AlphaUse alphapolicy;

  uint8_t * glcolors;

  int refcount;
  friend class nop; // to avoid g++ compiler warning on the private constructor
};

#endif // !COIN_CVRCLUT_H
