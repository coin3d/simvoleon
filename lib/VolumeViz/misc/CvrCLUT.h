#ifndef COIN_CVRCLUT_H
#define COIN_CVRCLUT_H

#include <Inventor/system/inttypes.h>
#include <Inventor/C/glue/gl.h>

class CvrCLUT {
public:
  CvrCLUT(const unsigned int nrcols, const uint8_t * colormap);
  CvrCLUT(const unsigned int nrcols, const unsigned int nrcomponents,
          const float * colormap);
  ~CvrCLUT();

  void setTransparencyThresholds(uint32_t low, uint32_t high);

  void activate(const cc_glglue * glw) const;
  void lookupRGBA(const unsigned int idx, uint8_t rgba[4]) const;

private:
  void commonConstructor(void);

  unsigned int nrentries;
  unsigned int nrcomponents;

  enum DataType { INTS, FLOATS } datatype;
  union {
    uint8_t * int_entries;
    float * flt_entries;
  };

  uint32_t transparencythresholds[2];
  uint8_t * transparentblock;
  int transparentblockentries;
};

#endif // !COIN_CVRCLUT_H
