#ifndef COIN_SOVOLUMEREADER_H
#define COIN_SOVOLUMEREADER_H

#include <VolumeViz/nodes/SoVolumeData.h>
#include <VolumeViz/nodes/SoOrthoSlice.h>

class SbBox2s;
class SbBox3f;
class SbVec3s;


class SoVolumeReader {
public:
  SoVolumeReader(void);
  virtual ~SoVolumeReader();

  virtual void setUserData(void * data);
  virtual void getDataChar(SbBox3f & size, SoVolumeData::DataType & type,
                           SbVec3s & dim) = 0;
  virtual void getSubSlice(SbBox2s &subSlice, int sliceNumber, void * data,
                           // FIXME: this is an extra parameter vs
                           // TGS. Investigate why. 20021107 mortene.
                           SoOrthoSlice::Axis axis = SoOrthoSlice::Z)
    = 0;

protected:
  int setFilename(const char * filename);
  void * getBuffer(int64_t offset, unsigned int size);
  int bytesToInt(unsigned char * ptr, int sizeBytes);
  void swapBytes(int * intPtr, int sizeBytes);
  int64_t fileSize();

  void * data;

private:
  friend class SoVolumeReaderP;
  class SoVolumeReaderP * pimpl;
};

#endif // !COIN_SOVOLUMEREADER_H
