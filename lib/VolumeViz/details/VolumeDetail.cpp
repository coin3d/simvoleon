/**************************************************************************\
 *
 *  This file is part of the SIM Voleon visualization library.
 *  Copyright (C) 2003-2004 by Systems in Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  ("GPL") version 2 as published by the Free Software Foundation.
 *  See the file LICENSE.GPL at the root directory of this source
 *  distribution for additional information about the GNU GPL.
 *
 *  For using SIM Voleon with software that can not be combined with
 *  the GNU GPL, and for taking advantage of the additional benefits
 *  of our support services, please contact Systems in Motion about
 *  acquiring a SIM Voleon Professional Edition License.
 *
 *  See <URL:http://www.coin3d.org/> for more information.
 *
 *  Systems in Motion, Teknobyen, Abels Gate 5, 7030 Trondheim, NORWAY.
 *  <URL:http://www.sim.no/>.
 *
\**************************************************************************/

/*!
  \class SoVolumeDetail SoVolumeDetail.h VolumeViz/details/SoVolumeDetail.h
  \brief The SoVolumeDetail stores ray intersection information through a volume.

  An SoVolumeDetail contains the information about a ray
  intersecting of a volume rendered with the SoVolumeRender node.

  The detail contains a "profile" of voxel values through the volume,
  where the profile is defined by a start point and an end point.
*/

#include <VolumeViz/details/SoVolumeDetail.h>
#include <Inventor/SbName.h>
#include <stddef.h>
#include <string.h>

// *************************************************************************

SO_DETAIL_SOURCE(SoVolumeDetail);

// *************************************************************************

SoVolumeDetail::SoVolumeDetail(void)
{
  assert(SoVolumeDetail::getClassTypeId() != SoType::badType());
}

SoVolumeDetail::~SoVolumeDetail()
{
}

// doc in super
void
SoVolumeDetail::initClass(void)
{
  SO_DETAIL_INIT_CLASS(SoVolumeDetail, SoDetail);
}

// doc in super
SoDetail *
SoVolumeDetail::copy(void) const
{
  SoVolumeDetail * copy = new SoVolumeDetail();
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
SoVolumeDetail::getProfileObjectPos(SbVec3f profile[2]) const
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

  If \a profile is \c NULL, the points will not be set, but the
  correct number of total profile points will still be returned.
*/
int
SoVolumeDetail::getProfileDataPos(SbVec3s profile[2]) const
{
  const unsigned int nrprofilepoints = this->voxelinfolist.getLength();
  assert(nrprofilepoints >= 2);

  if (profile != NULL) {
    profile[0] = this->voxelinfolist[0].voxelindex;
    profile[1] = this->voxelinfolist[nrprofilepoints - 1].voxelindex;
  }

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
SoVolumeDetail::getProfileValue(int index,
                                      SbVec3s * pos, SbVec3f * objpos,
                                      SbBool flag) const
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
SoVolumeDetail::getFirstNonTransparentValue(unsigned int * value,
                                                  SbVec3s * pos,
                                                  SbVec3f * objpos,
                                                  SbBool flag) const
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
SoVolumeDetail::addVoxelIntersection(const SbVec3f & voxelcoord,
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
