/*!
  \class SoOrthoSliceDetail SoOrthoSliceDetail.h VolumeViz/details/SoOrthoSliceDetail.h
  \brief The SoOrthoSliceDetail FIXME: doc

  FIXME: doc
*/

#include <stddef.h>

#include <VolumeViz/details/SoOrthoSliceDetail.h>

#include <Inventor/SbName.h>
#include <Inventor/SbVec3f.h>
#include <Inventor/SbVec3s.h>


SO_DETAIL_SOURCE(SoOrthoSliceDetail);


SoOrthoSliceDetail::SoOrthoSliceDetail(void)
{
  assert(SoOrthoSliceDetail::getClassTypeId() != SoType::badType());
}

SoOrthoSliceDetail::~SoOrthoSliceDetail()
{
}

// doc in super
void
SoOrthoSliceDetail::initClass(void)
{
  SO_DETAIL_INIT_CLASS(SoOrthoSliceDetail, SoDetail);
}

// doc in super
SoDetail *
SoOrthoSliceDetail::copy(void) const
{
  SoOrthoSliceDetail * copy = new SoOrthoSliceDetail();
  // FIXME: copy internal data
  return copy;
}

SbVec3f &
SoOrthoSliceDetail::getValueObjectPos(void) const
{
  // FIXME: implement
  static SbVec3f v;
  return v;
}

SbVec3s &
SoOrthoSliceDetail::getValueDataPos(void) const
{
  // FIXME: implement
  static SbVec3s v;
  return v;
}

unsigned int
SoOrthoSliceDetail::getValue(void) const
{
  // FIXME: implement
  return 0;
}
