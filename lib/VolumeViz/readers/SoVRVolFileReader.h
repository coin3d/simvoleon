#ifndef COIN_SOVRVOLFILEREADER_H
#define COIN_SOVRVOLFILEREADER_H

#include <VolumeViz/readers/SoVolumeReader.h>


class SoVRVolFileReader : public SoVolumeReader {
  typedef SoVolumeReader inherited;

public:
  SoVRVolFileReader(void);
  ~SoVRVolFileReader();

  void setUserData(void * data);
  void getDataChar(SbBox3f & size, SoVolumeData::DataType & type, SbVec3s & dim);
  virtual void getSubSlice(SbBox2s & subslice, int slicenumber, void * data);

private:
  class SoVRVolFileReaderP * pimpl;
  friend class SoVRVolFileReaderP;
};

#endif // ! COIN_SOVRVOLFILEREADER_H
