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
 *  Systems in Motion, Postboks 1283, Pirsenteret, 7462 Trondheim, NORWAY.
 *  <URL:http://www.sim.no/>.
 *
\**************************************************************************/

/*!
  \class SoOrthoSliceDetail SoOrthoSliceDetail.h VolumeViz/details/SoOrthoSliceDetail.h
  \brief The SoOrthoSliceDetail contains details of a pick operation on SoOrthoSlice geometry.
*/

// *************************************************************************

#include <stddef.h>
#include <VolumeViz/details/SoOrthoSliceDetail.h>
#include <Inventor/SbName.h>

// *************************************************************************

SO_DETAIL_SOURCE(SoOrthoSliceDetail);

// *************************************************************************

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

// *************************************************************************

// doc in super
SoDetail *
SoOrthoSliceDetail::copy(void) const
{
  SoOrthoSliceDetail * copy = new SoOrthoSliceDetail();

  copy->objectcoords = this->objectcoords;
  copy->ijkcoords = this->ijkcoords;
  copy->voxelvalue = this->voxelvalue;

  return copy;
}

// *************************************************************************

/*!
  Returns unit coordinates of the pick point, in the volume's local
  coordinate system.

  \sa getValueDataPos()
*/
const SbVec3f &
SoOrthoSliceDetail::getValueObjectPos(void) const
{
  return this->objectcoords;
}

/*!
  Returns coordinates of the voxel at the pick point, in the volume's
  voxel coordinate system.

  \sa getValueObjectPos(), getValue()
*/
const SbVec3s &
SoOrthoSliceDetail::getValueDataPos(void) const
{
  return this->ijkcoords;
}

/*!
  Returns value of the picked voxel.
*/
unsigned int
SoOrthoSliceDetail::getValue(void) const
{
  return this->voxelvalue;
}

// *************************************************************************
