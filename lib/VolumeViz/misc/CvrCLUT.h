#ifndef COIN_CVRCLUT_H
#define COIN_CVRCLUT_H

#include <Inventor/system/inttypes.h>

class CvrCLUT {
public:
  CvrCLUT(const unsigned int nrentries, uint8_t * rgba8bits);
  ~CvrCLUT();

private:
  unsigned int nrentries;
  uint8_t * entries;
};

#endif // !COIN_CVRCLUT_H
