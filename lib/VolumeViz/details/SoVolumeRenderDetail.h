#ifndef COIN_SOVOLUMERENDERDETAIL_H
#define COIN_SOVOLUMERENDERDETAIL_H

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

#include <Inventor/details/SoDetail.h>
#include <Inventor/details/SoSubDetail.h>
#include <Inventor/SbVec3f.h>
#include <Inventor/SbVec3s.h>
#include <Inventor/lists/SbList.h>
#include <VolumeViz/C/basic.h>


class SIMVOLEON_DLL_API SoVolumeRenderDetail : public SoDetail {
  typedef SoDetail inherited;

  SO_DETAIL_HEADER(SoVolumeRenderDetail);

public:
  static void initClass(void);
  SoVolumeRenderDetail(void);
  virtual ~SoVolumeRenderDetail();
 
  virtual SoDetail * copy(void) const;

  void getProfileObjectPos(SbVec3f profile[2]) const;
  int getProfileDataPos(SbVec3s profile[2] = 0) const;
  unsigned int getProfileValue(int index,
                               SbVec3s * pos = 0, SbVec3f * objpos = 0,
                               SbBool flag = FALSE) const;

  SbBool getFirstNonTransparentValue(unsigned int * value,
                                     SbVec3s * pos = 0, SbVec3f * objpos = 0,
                                     SbBool flag = FALSE) const;

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
