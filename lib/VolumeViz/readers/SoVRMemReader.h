#ifndef COIN_SOVRMEMREADER_H
#define COIN_SOVRMEMREADER_H

#include <VolumeViz/readers/SoVolumeReader.h>
#include <Inventor/SbBox3f.h>

class SoVRMemReader : public SoVolumeReader{
public:
  SoVRMemReader(void);
  virtual ~SoVRMemReader();

  void setUserData(void * data);

  void getDataChar(SbBox3f &size, SoVolumeData::DataType &type,
                   SbVec3s &dim);

  void getSubSlice(SbBox2s &subSlice, int sliceNumber, void * data,
                   Axis axis = Z);

  void setData(const SbVec3s &dimensions, void * data,
               const SbBox3f &volumeSize,
               SoVolumeData::DataType type = SoVolumeData::UNSIGNED_BYTE);

  void setVolumeSize(const SbBox3f &size);

private:
  friend class SoVRMemReaderP;
  class SoVRMemReaderP * pimpl;
};

#endif // !COIN_SOVOLUMEREADER_H
