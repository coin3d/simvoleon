#ifndef COIN_SOORTHOSLICEDETAIL_H
#define COIN_SOORTHOSLICEDETAIL_H

#include <Inventor/details/SoDetail.h>
#include <Inventor/details/SoSubDetail.h>

class SbVec3f;
class SbVec3s;


class SoOrthoSliceDetail : public SoDetail {
  typedef SoDetail inherited;

  SO_DETAIL_HEADER(SoOrthoSliceDetail);

public:
  static void initClass(void);
  SoOrthoSliceDetail(void);
  virtual ~SoOrthoSliceDetail();
 
  virtual SoDetail * copy(void) const;

  SbVec3f & getValueObjectPos(void) const;
  SbVec3s & getValueDataPos(void) const;
  unsigned int getValue(void) const;
};

#endif // !COIN_SOORTHOSLICEDETAIL_H
