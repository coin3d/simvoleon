#ifndef COIN_SOVOLUMERENDERDETAIL_H
#define COIN_SOVOLUMERENDERDETAIL_H

#include <Inventor/details/SoDetail.h>
#include <Inventor/details/SoSubDetail.h>
#include <Inventor/SbVec3f.h>
#include <Inventor/SbVec3s.h>
#include <Inventor/lists/SbList.h>


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

private:
  void addVoxelIntersection(const SbVec3f & voxelcoord,
                            const SbVec3s & voxelindex,
                            unsigned int voxelvalue,
                            uint8_t rgba[4]);

  class VoxelInfo {
  public:
    SbVec3f voxelcoord;
    SbVec3s voxelindex;
    unsigned int voxelvalue;
    uint8_t rgba[4];
  };
  SbList<VoxelInfo> voxelinfolist;

  friend class SoVolumeRender;
};

#endif // !COIN_SOVOLUMERENDERDETAIL_H
