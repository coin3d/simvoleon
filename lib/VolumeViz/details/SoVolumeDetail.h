#ifndef COIN_SOVOLUMEDETAIL_H
#define COIN_SOVOLUMEDETAIL_H

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


#include <Inventor/misc/SoState.h>
#include <Inventor/nodes/SoNode.h>
#include <Inventor/details/SoDetail.h>
#include <Inventor/details/SoSubDetail.h>
#include <Inventor/SbLinear.h>

#include <VolumeViz/C/basic.h>

class SIMVOLEON_DLL_API SoVolumeDetail : public SoDetail {
  typedef SoDetail inherited;

  SO_DETAIL_HEADER(SoVolumeDetail);

public:
  static void initClass(void);
  SoVolumeDetail(void);
  virtual ~SoVolumeDetail();
 
  virtual SoDetail * copy(void) const;

  void getProfileObjectPos(SbVec3f profile[2]) const;
  int getProfileDataPos(SbVec3s profile[2] = 0) const;
  unsigned int getProfileValue(int index,
                               SbVec3s * pos = 0, SbVec3f * objpos = 0,
                               SbBool flag = FALSE) const;

  SbBool getFirstNonTransparentValue(unsigned int * value,
                                     SbVec3s * pos = 0, SbVec3f * objpos = 0,
                                     SbBool flag = FALSE) const;

  // NOTE: The TGS version of the 'setDetails()' takes different
  // arguments. We consider their solution to be unoptimal, and have
  // therefore changed the input arguments.
  void setDetails(const SbVec3f raystart, const SbVec3f rayend, 
                  SoState * state, SoNode * caller);

private:

  class SoVolumeDetailP * pimpl;
  friend class SoVolumeDetailP;

};

#endif // !COIN_SOVOLUMEDETAIL_H
