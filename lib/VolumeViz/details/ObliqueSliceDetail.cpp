/*!
  \class SoObliqueSliceDetail SoObliqueSliceDetail.h VolumeViz/details/SoObliqueSliceDetail.h
  \brief The SoObliqueSliceDetail FIXME: doc

  FIXME: doc
*/

#include <stddef.h>
#include <VolumeViz/details/SoObliqueSliceDetail.h>
#include <Inventor/SbName.h>


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
  // internal data automatically copied
  return copy;
}

const SbVec3f &
SoObliqueSliceDetail::getValueObjectPos(void) const
{
  return this->objectcoords;
}

const SbVec3s &
SoObliqueSliceDetail::getValueDataPos(void) const
{
  return this->ijkcoords;
}

unsigned int
SoObliqueSliceDetail::getValue(void) const
{
  return this->voxelvalue;
}
