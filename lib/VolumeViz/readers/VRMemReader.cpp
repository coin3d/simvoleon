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
