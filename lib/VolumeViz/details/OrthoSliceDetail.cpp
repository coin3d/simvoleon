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
  \class SoOrthoSliceDetail SoOrthoSliceDetail.h VolumeViz/details/SoOrthoSliceDetail.h
  \brief The SoOrthoSliceDetail FIXME: doc

  FIXME: doc
*/

#include <stddef.h>
#include <VolumeViz/details/SoOrthoSliceDetail.h>
#include <Inventor/SbName.h>


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
  // internal data is automatically copied
  return copy;
}

const SbVec3f &
SoOrthoSliceDetail::getValueObjectPos(void) const
{
  return this->objectcoords;
}

const SbVec3s &
SoOrthoSliceDetail::getValueDataPos(void) const
{
  return this->ijkcoords;
}

unsigned int
SoOrthoSliceDetail::getValue(void) const
{
  return this->voxelvalue;
}
