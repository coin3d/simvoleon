/**************************************************************************\
 * 
 *   Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 * 
 *   Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *   http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 * 
\**************************************************************************/


#ifndef COIN_SOVRMEMREADER_H
#define COIN_SOVRMEMREADER_H

#include <VolumeViz/readers/SoVolumeReader.h>

class SoVRMemReader : public SoVolumeReader{
public:
  SoVRMemReader();
  ~SoVRMemReader();
  void setUserData(void * data);
  void getDataChar(SbBox3f &size, SoVolumeData::DataType &type, SbVec3s &dim);
  void getSubSlice( SbBox2s &subSlice, 
                    int sliceNumber, 
                    void * data, 
                    SoVolumeData::Axis axis);

  void setData(const SbVec3s &dimension, 
               const void *data, 
               SoVolumeData::DataType type=SoVolumeData::UNSIGNED_BYTE);
private:
  friend class SoVRMemReader;
  class SoVRMemReaderP * pimpl;
};//SoVolumeReader

#endif // !COIN_SOVOLUMEREADER_H
