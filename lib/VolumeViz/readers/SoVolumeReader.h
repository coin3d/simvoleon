#ifndef COIN_SOVOLUMEREADER_H
#define COIN_SOVOLUMEREADER_H

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

#include <VolumeViz/nodes/SoVolumeData.h>

class SbBox2s;
class SbBox3f;
class SbVec3s;


class SIMVOLEON_DLL_API SoVolumeReader {
public:
  SoVolumeReader(void);
  virtual ~SoVolumeReader();

  virtual void setUserData(void * data);
  virtual void getDataChar(SbBox3f & size, SoVolumeData::DataType & type,
                           SbVec3s & dim) = 0;

  virtual void getSubSlice(SbBox2s & subslice, int slicenumber, void * data) = 0;

protected:
  int setFilename(const char * filename);
  void * getBuffer(int64_t offset, unsigned int size);
  int bytesToInt(unsigned char * ptr, int sizeBytes);
  void swapBytes(int * intPtr, int sizeBytes);
  int64_t fileSize(void);

  void * m_data;

private:
  friend class SoVolumeReaderP;
  class SoVolumeReaderP * pimpl;

  // FIXME: ugly design. 20021120 mortene.
  friend class SoVolumeData; // For m_data access.
};

#endif // !COIN_SOVOLUMEREADER_H
