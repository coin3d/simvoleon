#ifndef COIN_SOORTHOSLICEDETAIL_H
#define COIN_SOORTHOSLICEDETAIL_H

#include <Inventor/details/SoDetail.h>
#include <Inventor/details/SoSubDetail.h>
#include <Inventor/SbVec3f.h>
#include <Inventor/SbVec3s.h>
#include <VolumeViz/C/basic.h>


class SIMVOLEON_DLL_API SoOrthoSliceDetail : public SoDetail {
  typedef SoDetail inherited;

  SO_DETAIL_HEADER(SoOrthoSliceDetail);

public:
  static void initClass(void);
  SoOrthoSliceDetail(void);
  virtual ~SoOrthoSliceDetail();
 
  virtual SoDetail * copy(void) const;

  const SbVec3f & getValueObjectPos(void) const;
  const SbVec3s & getValueDataPos(void) const;
  unsigned int getValue(void) const;

private:
  SbVec3f objectcoords;
  SbVec3s ijkcoords;
  unsigned int voxelvalue;

  friend class SoOrthoSlice;
};

#endif // !COIN_SOORTHOSLICEDETAIL_H
