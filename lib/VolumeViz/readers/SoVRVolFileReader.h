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
  virtual void getSubSlice(SbBox2s & subslice, int slicenumber, void * data,
                           // FIXME: this is an extra argument vs
                           // TGS. Explained in
                           // SoVolumeData.cpp. Consider if it's
                           // really worth extending the API
                           // for. 20021107 mortene.
                           Axis axis = Z);


private:
  class SoVRVolFileReaderP * pimpl;
  friend class SoVRVolFileReaderP;
};

#endif // ! COIN_SOVRVOLFILEREADER_H
