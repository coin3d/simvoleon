/*!
  \class SoObliqueSliceDetail SoObliqueSliceDetail.h VolumeViz/details/SoObliqueSliceDetail.h
  \brief The SoObliqueSliceDetail FIXME: doc

  FIXME: doc
*/

#include <stddef.h>

#include <VolumeViz/details/SoObliqueSliceDetail.h>

#include <Inventor/SbName.h>
#include <Inventor/SbVec3f.h>
#include <Inventor/SbVec3s.h>


SO_DETAIL_SOURCE(SoObliqueSliceDetail);


SoObliqueSliceDetail::SoObliqueSliceDetail(void)
{
  assert(SoObliqueSliceDetail::getClassTypeId() != SoType::badType());
}

SoObliqueSliceDetail::~SoObliqueSliceDetail()
{
}

// doc in super
void
SoObliqueSliceDetail::initClass(void)
{
  SO_DETAIL_INIT_CLASS(SoObliqueSliceDetail, SoDetail);
}

// doc in super
SoDetail *
SoObliqueSliceDetail::copy(void) const
{
  SoObliqueSliceDetail * copy = new SoObliqueSliceDetail();
  // FIXME: copy internal data
  return copy;
}

SbVec3f &
SoObliqueSliceDetail::getValueObjectPos(void) const
{
  // FIXME: implement
  static SbVec3f v;
  return v;
}

SbVec3s &
SoObliqueSliceDetail::getValueDataPos(void) const
{
  // FIXME: implement
  static SbVec3s v;
  return v;
}

unsigned int
SoObliqueSliceDetail::getValue(void) const
{
  // FIXME: implement
  return 0;
}
