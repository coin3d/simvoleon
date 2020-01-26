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
  \class SoVolumeReader VolumeViz/readers/SoVolumeReader.h
  \brief Abstract superclass for all volume data reader classes.
*/

// *************************************************************************

#include <VolumeViz/readers/SoVolumeReader.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif // HAVE_UNISTD_H

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif // HAVE_SYS_TYPES_H

#include <sys/stat.h>
#include <errno.h>
#include <string.h>

#include <Inventor/errors/SoDebugError.h>

// *************************************************************************

/*!
  \fn void SoVolumeReader::getDataChar(SbBox3f & size, SoVolumeData::DataType & type, SbVec3s & dim)

  Returns information about the volume data contained in a volume
  reader. Sub-classes, i.e. the non-abstract readers, need to
  implement this function.

  \a size is set to the "world size" of the volume, in unit
  coordinates.

  \a type is set to either SoVolumeData::UNSIGNED_BYTE or
  SoVolumeData::UNSIGNED_SHORT, to signify that the voxel values are
  either 8-bit or 16-bit, respectively.

  \a dim gives the volume dimensions in voxel coordinates, i.e. the
  number of rows, columns and stacks of voxels along the internal 3
  coordinate axes of the volume.
*/

/*!
  \fn void SoVolumeReader::getSubSlice(SbBox2s & subslice, int slicenumber, void * data)

  Extract a subslice from the volume (which may still reside solely on
  disk). Sub-classes, i.e. the non-abstract readers, need to
  implement this function.
*/

// *************************************************************************

class SoVolumeReaderP {
public:
  SoVolumeReaderP(SoVolumeReader * master) {
    this->master = master;
  }

  SbString filename;

private:
  SoVolumeReader * master;
};


#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

// *************************************************************************

SoVolumeReader::SoVolumeReader(void)
{
  PRIVATE(this) = new SoVolumeReaderP(this);
  this->m_data = NULL;
}

SoVolumeReader::~SoVolumeReader()
{
  // FIXME: dealloc this->m_data when we allocated it
  // ourselves. 20021109 mortene.

  delete PRIVATE(this);
}

void
SoVolumeReader::setUserData(void * data)
{
  // FIXME: unimplemented. 20021108 mortene.
}

// *************************************************************************

int
SoVolumeReader::setFilename(const char * filename)
{
  PRIVATE(this)->filename = filename;

  // TGS doc doesn't say what this is supposed to return.
  return 0;
}

int64_t
SoVolumeReader::fileSize(void)
{
  struct stat buf;
  const char * filename = PRIVATE(this)->filename.getString();

  if (stat(filename, &buf) == 0) {
    return buf.st_size;
  }
  else {
    SoDebugError::post("SoVolumeReader::fileSize", "couldn't stat() '%s': %s",
                       filename, strerror(errno));
    return -1;
  }
}

// FIXME: unimplemented methods follows. 20021108 mortene.

void *
SoVolumeReader::getBuffer(int64_t offset, unsigned int size)
{
  return NULL;
}

int SoVolumeReader::bytesToInt(unsigned char * ptr, int sizebytes) { return 0x0000; }
void SoVolumeReader::swapBytes(int * intptr, int sizebytes) { return; }

// *************************************************************************

// \since SIM Voleon 2.0
int
SoVolumeReader::getNumSignificantBits(void)
{
  // FIXME: implement. 20041008 mortene.
  return 0;
}

// *************************************************************************

// \since SIM Voleon 2.0
SbBool
SoVolumeReader::getSubVolume(SbBox3s & volume, void * data)
{
  // FIXME: implement in SoVRVolFileReader, plus use it from within
  // the library when loading blocks. 20041008 mortene.

  // FALSE means fall back on getSubSlice().
  return FALSE;
}

// \since SIM Voleon 2.0
SbBool
SoVolumeReader::getSubVolume(const SbBox3s & volume,
                             const SbVec3s subsamplelevel, void *& voxels)
{
  // FIXME: implement in SoVRVolFileReader, plus use it from within
  // the library when loading blocks. 20041008 mortene.

  // FALSE means fall back on getSubSlice().
  return FALSE;
}

// \since SIM Voleon 2.0
SbBool
SoVolumeReader::getSubVolumeInfo(SbBox3s & volume,
                                 SbVec3s reqsubsamplelevel,
                                 SbVec3s & subsamplelevel,
                                 SoVolumeReader::CopyPolicy & policy)
{
  // FIXME: implement. 20041008 mortene.
  return FALSE;
}

// *************************************************************************

// \since SIM Voleon 2.0
SbVec3s
SoVolumeReader::getNumVoxels(SbVec3s realsize, SbVec3s subsamplinglevel) const
{
  // FIXME: implement. 20041008 mortene.

  SoDebugError::postWarning("SoVolumeReader::getNumVoxels",
                            "not yet implemented, just a stub");
  return SbVec3s(0, 0, 0);
}

// \since SIM Voleon 2.0
SbVec3s
SoVolumeReader::getSizeToAllocate(SbVec3s realsize, SbVec3s subsamplinglevel) const
{
  // FIXME: implement. 20041008 mortene.

  SoDebugError::postWarning("SoVolumeReader::getSizeToAllocate",
                            "not yet implemented, just a stub");
  return SbVec3s(0, 0, 0);
}

// *************************************************************************
