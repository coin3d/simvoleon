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
  \class SoObliqueSliceDetail SoObliqueSliceDetail.h VolumeViz/details/SoObliqueSliceDetail.h
  \brief The SoObliqueSliceDetail contains details of a pick operation on SoObliqueSlice geometry.

  \since SIM Voleon 1.1
*/

// *************************************************************************

#include <stddef.h>
#include <VolumeViz/details/SoObliqueSliceDetail.h>
#include <Inventor/SbName.h>

// *************************************************************************

SO_DETAIL_SOURCE(SoObliqueSliceDetail);

// *************************************************************************

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

// *************************************************************************

// doc in super
SoDetail *
SoObliqueSliceDetail::copy(void) const
{
  SoObliqueSliceDetail * copy = new SoObliqueSliceDetail();
  // internal data automatically copied
  return copy;
}

// *************************************************************************

/*!
  Returns unit coordinates of the pick point, in the volume's local
  coordinate system.

  \sa getValueDataPos()
*/
const SbVec3f &
SoObliqueSliceDetail::getValueObjectPos(void) const
{
  return this->objectcoords;
}

/*!
  Returns coordinates of the voxel at the pick point, in the volume's
  voxel coordinate system.

  \sa getValueObjectPos(), getValue()
*/
const SbVec3s &
SoObliqueSliceDetail::getValueDataPos(void) const
{
  return this->ijkcoords;
}

/*!
  Returns value of the picked voxel.
*/
unsigned int
SoObliqueSliceDetail::getValue(void) const
{
  return this->voxelvalue;
}

// *************************************************************************
