/*!
  \class SoVolumeRenderDetail SoVolumeRenderDetail.h VolumeViz/details/SoVolumeRenderDetail.h
  \brief The SoVolumeRenderDetail FIXME: doc

  FIXME: doc
*/

#include <VolumeViz/details/SoVolumeRenderDetail.h>
#include <Inventor/SbName.h>
#include <stddef.h>

SO_DETAIL_SOURCE(SoVolumeRenderDetail);

SoVolumeRenderDetail::SoVolumeRenderDetail(void)
{
  assert(SoVolumeRenderDetail::getClassTypeId() != SoType::badType());
}

SoVolumeRenderDetail::~SoVolumeRenderDetail()
{
}

// doc in super
void
SoVolumeRenderDetail::initClass(void)
{
  SO_DETAIL_INIT_CLASS(SoVolumeRenderDetail, SoDetail);
}

// doc in super
SoDetail *
SoVolumeRenderDetail::copy(void) const
{
  SoVolumeRenderDetail * copy = new SoVolumeRenderDetail();
  // FIXME: copy internal data
  return copy;
}

void
SoVolumeRenderDetail::getProfileObjectPos(SbVec3f profile[2])
{
  // FIXME: implement
}

int
SoVolumeRenderDetail::getProfileDataPos(SbVec3s profile[2])
{
  // FIXME: implement
  return 0;
}

unsigned int
SoVolumeRenderDetail::getProfileValue(int index,
                                      SbVec3s * pos, SbVec3f * objpos,
                                      SbBool flag)
{
  // FIXME: implement
  return 0;
}

SbBool
SoVolumeRenderDetail::getFirstNonTransparentValue(unsigned int * value,
                                                  SbVec3s * pos,
                                                  SbVec3f * objpos,
                                                  SbBool flag)
{
  // FIXME: implement
  return FALSE;
}
