/*!
  \class SoVolumeRenderDetail SoVolumeRenderDetail.h VolumeViz/details/SoVolumeRenderDetail.h
  \brief The SoVolumeRenderDetail stores ray intersection information through a volume.

  An SoVolumeRenderDetail contains the information about a ray
  intersecting of a volume rendered with the SoVolumeRender node.

  The detail contains a "profile" of voxel values through the volume,
  where the profile is defined by a start point and an end point.
*/

#include <VolumeViz/details/SoVolumeRenderDetail.h>
#include <Inventor/SbName.h>
#include <stddef.h>

// *************************************************************************

SO_DETAIL_SOURCE(SoVolumeRenderDetail);

// *************************************************************************

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
  // internal data is automatically copied
  return copy;
}

/*!
  Sets start and end points of ray intersecting the volume in the
  \a profile argument. The points returned are in object-space
  coordinates (i.e. in the same geometry unit system as polygon-based
  geometry in a scene).
*/
void
SoVolumeRenderDetail::getProfileObjectPos(SbVec3f profile[2])
{
  const unsigned int nrprofilepoints = this->voxelinfolist.getLength();
  assert(nrprofilepoints >= 2);
  profile[0] = this->voxelinfolist[0].voxelcoord;
  profile[1] = this->voxelinfolist[nrprofilepoints - 1].voxelcoord;
}

/*!
  Sets start and end points of ray intersecting the volume in the \a
  profile argument. The points returned are in voxel-space
  coordinates (i.e. within the dimensions of the voxel block).

  Returns total number of voxels intersected and stored in the
  profile. This can be used to iterate over the full profile with the
  \a getProfileValue() method.
*/
int
SoVolumeRenderDetail::getProfileDataPos(SbVec3s profile[2])
{
  const unsigned int nrprofilepoints = this->voxelinfolist.getLength();
  assert(nrprofilepoints >= 2);
  profile[0] = this->voxelinfolist[0].voxelindex;
  profile[1] = this->voxelinfolist[nrprofilepoints - 1].voxelindex;

  return nrprofilepoints;
}

/*!
  Returns voxel value at the given index along the ray intersection
  profile.

  If \a pos is not \c NULL, sets \a pos argument to voxel-space
  coordinates for voxel.

  If \a objpos is not \c NULL, sets \a objpos argument to object-space
  coordinates for voxel.

  The \a flag argument is unused and only included for compatibility
  with TGS Inventor's VolumeViz extension. (Where it seems to be
  undocumented.)
*/
unsigned int
SoVolumeRenderDetail::getProfileValue(int index,
                                      SbVec3s * pos, SbVec3f * objpos,
                                      SbBool flag)
{
  assert(index >= 0 && index < this->voxelinfolist.getLength());

  if (pos) { *pos = this->voxelinfolist[index].voxelindex; }
  if (objpos) { *objpos = this->voxelinfolist[index].voxelcoord; }
  return this->voxelinfolist[index].voxelvalue;
}

/*!
  Fills in the information about the first voxel along the pick ray
  intersection which is not completely transparent.

  Returns \c FALSE if all voxels along pick ray was fully transparent,
  otherwise \c TRUE.

  Sets \a value to the value of the voxel.

  If \a pos is not \c NULL, sets \a pos argument to voxel-space
  coordinates for the first voxel with some opaqueness.

  If \a objpos is not \c NULL, sets \a objpos argument to object-space
  coordinates for the first voxel with some opaqueness.

  The \a flag argument is unused and only included for compatibility
  with TGS Inventor's VolumeViz extension. (Where it seems to be
  undocumented.)
*/
SbBool
SoVolumeRenderDetail::getFirstNonTransparentValue(unsigned int * value,
                                                  SbVec3s * pos,
                                                  SbVec3f * objpos,
                                                  SbBool flag)
{
  int idx = 0;
  for (idx=0; idx < this->voxelinfolist.getLength(); idx++) {
    if (this->voxelinfolist[idx].rgba[3] != 0x00) break;
  }

  if (idx == this->voxelinfolist.getLength()) { return FALSE; }

  if (pos) { *pos = this->voxelinfolist[idx].voxelindex; }
  if (objpos) { *objpos = this->voxelinfolist[idx].voxelcoord; }
  if (value) { *value = this->voxelinfolist[idx].voxelvalue; }
  return TRUE;
}

void
SoVolumeRenderDetail::addVoxelIntersection(const SbVec3f & voxelcoord,
                                           const SbVec3s & voxelindex,
                                           unsigned int voxelvalue,
                                           uint8_t rgba[4])
{
  VoxelInfo vxinfo;
  vxinfo.voxelcoord = voxelcoord;
  vxinfo.voxelindex = voxelindex;
  vxinfo.voxelvalue = voxelvalue;
  (void)memcpy(vxinfo.rgba, rgba, sizeof(uint8_t) * 4);

  this->voxelinfolist.append(vxinfo);
}
