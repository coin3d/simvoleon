#ifndef COIN_CVRGIMPGRADIENT_H
#define COIN_CVRGIMPGRADIENT_H

#include <Inventor/system/inttypes.h>


class CvrGIMPGradientSegment {
public:
  float left, middle, right;
  float left_RGBA[4];
  float right_RGBA[4];
  int type;
  int color;
};

class CvrGIMPGradient {
public:
  int nrsegments;
  class CvrGIMPGradientSegment * segments;

  static CvrGIMPGradient * read(const char * buffer);
  void convertToIntArray(uint8_t intgradient[256][4]) const;
};

#endif // !COIN_CVRGIMPGRADIENT_H
