/**************************************************************************\
 * 
 *   Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 * 
 *   Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *   http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 * 
\**************************************************************************/


#ifndef COIN_SOVOLUMEREADER_H
#define COIN_SOVOLUMEREADER_H

#include <Inventor/nodes/SoNode.h>
#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/SbBox2f.h>
#include <Inventor/SbBox3f.h>
#include <Inventor/SbVec3s.h>
#include <Inventor/SbBox2s.h>
#include <VolumeViz/nodes/SoVolumeRendering.h>

class SoVolumeReader {
public:
  SoVolumeReader();
  ~SoVolumeReader();

  virtual void setUserData(void * data);
  virtual void getDataChar( SbBox3f &size, 
                            SoVolumeRendering::DataType &type, 
                            SbVec3s &dim) = 0;
  virtual void getSubSlice( SbBox2s &subSlice, 
                            int sliceNumber, 
                            void * data,
                            SoVolumeRendering::Axis axis = SoVolumeRendering::Z) 
                            = 0;

protected: 

  int setFilename(const char * filename);
  void *  getBuffer(int64_t offset, unsigned int size);
  int bytesToInt(unsigned char * ptr, int sizeBytes);
  void swapBytes(int * intPtr, int sizeBytes);
  int64_t fileSize();

  void * data;

private:
  friend class SoVolumeReaderP;
  class SoVolumeReaderP * pimpl;
};//SoVolumeReader

#endif // !COIN_SOVOLUMEREADER_H
