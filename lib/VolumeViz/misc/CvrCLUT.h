#ifndef COIN_CVRCLUT_H
#define COIN_CVRCLUT_H

#include <Inventor/system/inttypes.h>
#include <Inventor/C/glue/gl.h>

class CvrCLUT {
public:
  CvrCLUT(const unsigned int nrcols, const uint8_t * rgba8bits);
  CvrCLUT(const unsigned int nrcols, const unsigned int nrcomponents,
          const float * colormap);
  ~CvrCLUT();

  void activate(const cc_glglue * glw) const;

private:
  unsigned int nrentries;
  unsigned int nrcomponents;

  enum DataType { INTS, FLOATS } datatype;

  union {
    uint8_t * int_entries;
    float * flt_entries;
  };
};

#endif // !COIN_CVRCLUT_H
