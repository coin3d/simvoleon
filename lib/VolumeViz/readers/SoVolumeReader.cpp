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
  reader. Sub-classes, i.e. the non-abstract readers, needs to
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
  disk). Sub-classes, i.e. the non-abstract readers, needs to
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
