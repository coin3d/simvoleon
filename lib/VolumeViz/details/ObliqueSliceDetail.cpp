/**************************************************************************\
 * Copyright (c) Kongsberg Oil & Gas Technologies AS
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
\**************************************************************************/

/*!
  \class SoObliqueSliceDetail SoObliqueSliceDetail.h VolumeViz/details/SoObliqueSliceDetail.h
  \brief The SoObliqueSliceDetail contains details of a pick operation on SoObliqueSlice geometry.

  \since SIM Voleon 2.0
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
