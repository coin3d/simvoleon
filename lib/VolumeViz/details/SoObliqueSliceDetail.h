#ifndef COIN_SOOBLIQUESLICEDETAIL_H
#define COIN_SOOBLIQUESLICEDETAIL_H

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

// FIXME: class not yet in use. 20040505 mortene.


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
