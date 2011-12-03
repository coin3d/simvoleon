#ifndef COIN_SOVOLUMEREADER_H
#define COIN_SOVOLUMEREADER_H

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

#include <VolumeViz/nodes/SoVolumeData.h>

class SbBox2s;
class SbBox3f;
class SbVec3s;


class SIMVOLEON_DLL_API SoVolumeReader {
public:
  SoVolumeReader(void);
  virtual ~SoVolumeReader();

  virtual void setUserData(void * data);
  virtual int getNumSignificantBits(void);

  virtual void getDataChar(SbBox3f & size, SoVolumeData::DataType & type,
                           SbVec3s & dim) = 0;

  enum CopyPolicy { COPY, NO_COPY, NO_COPY_AND_DELETE };
  
  virtual void getSubSlice(SbBox2s & slice, int slicenumber, void * voxels) = 0;
  virtual SbBool getSubVolume(SbBox3s & volume, void * voxels);
  virtual SbBool getSubVolume(const SbBox3s & volume,
                              const SbVec3s subsamplelevel, void *& voxels);
  virtual SbBool getSubVolumeInfo(SbBox3s & volume,
                                  SbVec3s reqsubsamplelevel,
                                  SbVec3s & subsamplelevel,
                                  SoVolumeReader::CopyPolicy & policy);

  SbVec3s getNumVoxels(SbVec3s realsize, SbVec3s subsamplinglevel) const;
  SbVec3s getSizeToAllocate(SbVec3s realsize, SbVec3s subsamplinglevel) const;

  int setFilename(const char * filename);

protected:
  void * getBuffer(int64_t offset, unsigned int size);
  int bytesToInt(unsigned char * ptr, int sizeBytes);
  void swapBytes(int * intPtr, int sizeBytes);
  int64_t fileSize(void);

  void * m_data;

private:
  friend class SoVolumeReaderP;
  class SoVolumeReaderP * pimpl;

  // FIXME: SoVolumeData shouldn't really access m_data, as voxel data
  // should be stored within SoVolumeData, and not on this pointer.
  // 20041008 mortene.
  friend class SoVolumeData; // For m_data access.
};

#endif // !COIN_SOVOLUMEREADER_H
