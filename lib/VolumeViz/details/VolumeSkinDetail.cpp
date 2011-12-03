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
  \class SoVolumeSkinDetail SoVolumeSkinDetail.h VolumeViz/details/SoVolumeSkinDetail.h
  \brief The SoVolumeSkinDetail stores ray intersection information through a volume.

  An SoVolumeSkinDetail contains the information about a ray
  intersecting of a volume rendered with the SoVolumeSkin node.

  The detail contains a "profile" of voxel values through the volume,
  where the profile is defined by a start point and an end point.

  See the superclass documentation for information on how to extract
  the information about the ray profile.
*/

#include <VolumeViz/details/SoVolumeSkinDetail.h>
#include <Inventor/SbName.h>
#include <stddef.h>
#include <string.h>

// *************************************************************************

SO_DETAIL_SOURCE(SoVolumeSkinDetail);

// *************************************************************************

SoVolumeSkinDetail::SoVolumeSkinDetail(void)
{
  assert(SoVolumeSkinDetail::getClassTypeId() != SoType::badType());
}

SoVolumeSkinDetail::~SoVolumeSkinDetail()
{
}

// doc in super
void
SoVolumeSkinDetail::initClass(void)
{
  SO_DETAIL_INIT_CLASS(SoVolumeSkinDetail, SoDetail);
}
