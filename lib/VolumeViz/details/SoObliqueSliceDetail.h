#ifndef COIN_SOOBLIQUESLICEDETAIL_H
#define COIN_SOOBLIQUESLICEDETAIL_H

#include <Inventor/details/SoDetail.h>
#include <Inventor/details/SoSubDetail.h>
#include <Inventor/SbVec3f.h>
#include <Inventor/SbVec3s.h>
#include <VolumeViz/C/basic.h>


class SIMVOLEON_DLL_API SoObliqueSliceDetail : public SoDetail {
  typedef SoDetail inherited;

  SO_DETAIL_HEADER(SoObliqueSliceDetail);

public:
  static void initClass(void);
  SoObliqueSliceDetail(void);
  virtual ~SoObliqueSliceDetail();
 
  virtual SoDetail * copy(void) const;

  const SbVec3f & getValueObjectPos(void) const;
  const SbVec3s & getValueDataPos(void) const;
  unsigned int getValue(void) const;

private:
  SbVec3f objectcoords;
  SbVec3s ijkcoords;
  unsigned int voxelvalue;

  friend class SoObliqueSlice;
};

#endif // !COIN_SOOBLIQUESLICEDETAIL_H
