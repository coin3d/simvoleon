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

// FIXME: get rid of this class, as I don't think it adds anything of
// real value..? 20021126 mortene.
//
// UPDATE: voxel data should be stored in SoVolumeData, not in a(ny)
// reader. 20041008 mortene.

/*!
  \class SoVRMemReader VolumeViz/readers/SoVRMemReader.h
  \brief FIXME: doc
*/

// *************************************************************************

#include <VolumeViz/readers/SoVRMemReader.h>
#include <VolumeViz/misc/CvrUtil.h>
#include <VolumeViz/misc/CvrVoxelChunk.h>

#include <Inventor/errors/SoDebugError.h>

#include <string.h> // memcpy()


class SoVRMemReaderP {
public:
  SoVRMemReaderP(SoVRMemReader * master) {
    this->master = master;

    this->dimensions = SbVec3s(0, 0, 0);
    this->dataType = SoVolumeData::UNSIGNED_BYTE;
  }

  SbVec3s dimensions;
  SoVolumeData::DataType dataType;

private:
  SoVRMemReader * master;
};

#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

// *************************************************************************

SoVRMemReader::SoVRMemReader(void)
{
  PRIVATE(this) = new SoVRMemReaderP(this);
}

SoVRMemReader::~SoVRMemReader()
{
  delete PRIVATE(this);
}

void
SoVRMemReader::setUserData(void * data)
{
}

void SoVRMemReader::getDataChar(SbBox3f & size,
                                SoVolumeData::DataType & type,
                                SbVec3s & dim)
{
  type = PRIVATE(this)->dataType;
  dim = PRIVATE(this)->dimensions;

  const short largestdimension = SbMax(dim[0], SbMax(dim[1], dim[2]));
  SbVec3f normdims(dim[0], dim[1], dim[2]);
  normdims /= float(largestdimension);
  normdims *= 2.0f;
  size.setBounds(-normdims / 2.0f, normdims / 2.0f);
}

void
SoVRMemReader::getSubSlice(SbBox2s & subslice, int slicenumber, void * data)
{
  unsigned int bytesprvoxel;
  switch (PRIVATE(this)->dataType) {
  case SoVolumeData::UNSIGNED_BYTE: bytesprvoxel = 1; break;
  case SoVolumeData::UNSIGNED_SHORT: bytesprvoxel = 2; break;
  default: assert(FALSE); break;
  }

  // FIXME: interface of buildSubPage() should be improved to avoid
  // this roundabout way of clipping out a slice.  20021203 mortene.
  CvrVoxelChunk vc(PRIVATE(this)->dimensions, bytesprvoxel, this->m_data);
  CvrVoxelChunk * output = vc.buildSubPage(2 /* Z */, slicenumber, subslice);
  (void)memcpy(data, output->getBuffer(), output->bufferSize());
  delete output;
}


void
SoVRMemReader::setData(const SbVec3s &dimensions,
                       void * data,
                       SoVolumeData::DataType type)
{
  PRIVATE(this)->dimensions = dimensions;
  // FIXME: this is completely bogus use of SoVolumeReader::m_data --
  // this is *not* where the voxel data is supposed to be stored. That
  // is inside SoVolumeData. 20041008 mortene.
  this->m_data = data;
  PRIVATE(this)->dataType = type;
}
