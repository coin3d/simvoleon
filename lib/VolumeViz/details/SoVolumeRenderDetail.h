#ifndef COIN_SOVOLUMERENDERDETAIL_H
#define COIN_SOVOLUMERENDERDETAIL_H

#include <Inventor/details/SoDetail.h>
#include <Inventor/details/SoSubDetail.h>
#include <Inventor/SbVec3f.h>
#include <Inventor/SbVec3s.h>


class SoVolumeRenderDetail : public SoDetail {
  typedef SoDetail inherited;

  SO_DETAIL_HEADER(SoVolumeRenderDetail);

public:
  static void initClass(void);
  SoVolumeRenderDetail(void);
  virtual ~SoVolumeRenderDetail();
 
  virtual SoDetail * copy(void) const;

  void getProfileObjectPos(SbVec3f profile[2]);
  int getProfileDataPos(SbVec3s profile[2] = 0);
  unsigned int getProfileValue(int index,
                               SbVec3s * pos = 0, SbVec3f * objpos = 0,
                               SbBool flag = FALSE);

  SbBool getFirstNonTransparentValue(unsigned int * value,
                                     SbVec3s * pos = 0, SbVec3f * objpos = 0,
                                     SbBool flag = FALSE);
};

#endif // !COIN_SOVOLUMERENDERDETAIL_H
