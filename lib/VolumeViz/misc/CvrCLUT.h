#ifndef COIN_CVRCLUT_H
#define COIN_CVRCLUT_H

#include <Inventor/system/inttypes.h>
#include <Inventor/C/glue/gl.h>

class CvrCLUT {
public:
  CvrCLUT(const unsigned int nrentries, uint8_t * rgba8bits);
  ~CvrCLUT();

  void activate(const cc_glglue * glw) const;

private:
  unsigned int nrentries;
  uint8_t * entries;
};

#endif // !COIN_CVRCLUT_H
