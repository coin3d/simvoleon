#ifndef COIN_SOVRMEMREADER_H
#define COIN_SOVRMEMREADER_H

#include <VolumeViz/readers/SoVolumeReader.h>
#include <Inventor/SbBox3f.h>

class SoVRMemReader : public SoVolumeReader{
public:
  SoVRMemReader(void);
  virtual ~SoVRMemReader();

  void setUserData(void * data);

  void getDataChar(SbBox3f &size, SoVolumeRendering::DataType &type,
                   SbVec3s &dim);

  void getSubSlice(SbBox2s &subSlice, int sliceNumber, void * data,
                   SoVolumeRendering::Axis axis = SoVolumeRendering::Z);

  void setData(const SbVec3s &dimensions, const void *data,
               const SbBox3f &volumeSize,
               SoVolumeRendering::DataType type = SoVolumeRendering::UNSIGNED_BYTE);

  void setVolumeSize(const SbBox3f &size);

private:
  friend class SoVRMemReaderP;
  class SoVRMemReaderP * pimpl;
};

#endif // !COIN_SOVOLUMEREADER_H
