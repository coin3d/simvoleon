#ifndef COIN_CVRUTIL_H
#define COIN_CVRUTIL_H

#include <Inventor/SbBasic.h>


class CvrUtil {
public:
  static SbBool doDebugging(void);
  static SbBool debugRayPicks(void);

  static uint32_t crc32(uint8_t * buf, unsigned int len);
};

#endif // !COIN_CVRUTIL_H
