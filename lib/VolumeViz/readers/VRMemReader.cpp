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

// FIXME: get rid of this class, as I don't think it adds anything of
// real value..? 20021126 mortene.

/*!
  \class SoVRMemReader VolumeViz/readers/SoVRMemReader.h
  \brief FIXME: doc
  \ingroup volviz
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


void SoVRMemReader::setUserData(void * data)
{
}

void SoVRMemReader::getDataChar(SbBox3f & size,
                                SoVolumeData::DataType & type,
                                SbVec3s & dim)
{
  type = PRIVATE(this)->dataType;
  dim = PRIVATE(this)->dimensions;

  size.setBounds(-dim[0]/2.0f, -dim[1]/2.0f, -dim[2]/2.0f,
                 dim[0]/2.0f, dim[1]/2.0f, dim[2]/2.0f);
}

void
SoVRMemReader::getSubSlice(SbBox2s & subslice, int slicenumber, void * data)
{
  CvrVoxelChunk::UnitSize vctype;
  switch (PRIVATE(this)->dataType) {
  case SoVolumeData::UNSIGNED_BYTE: vctype = CvrVoxelChunk::UINT_8; break;
  case SoVolumeData::UNSIGNED_SHORT: vctype = CvrVoxelChunk::UINT_16; break;
  default: assert(FALSE); break;
  }

  // FIXME: interface of buildSubPage() should be improved to avoid
  // this roundabout way of clipping out a slice.  20021203 mortene.
  CvrVoxelChunk vc(PRIVATE(this)->dimensions, vctype, this->m_data);
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
  this->m_data = data;
  PRIVATE(this)->dataType = type;
}
