/*!
  \class SoVRMemReader VolumeViz/readers/SoVRMemReader.h
  \brief FIXME: doc
  \ingroup volviz
*/

#include <VolumeViz/readers/SoVRMemReader.h>

#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H


class SoVRMemReaderP{
public:
  SoVRMemReaderP(SoVRMemReader * master) {
    this->master = master;

    data = NULL;
    dimensions = SbVec3s(0, 0, 0);
    dataType = SoVolumeData::UNSIGNED_BYTE;
  }

  SbVec3s dimensions;
  const void * data;
  SoVolumeData::DataType dataType;
  SbBox3f volumeSize;

  void buildSubSliceX(void * output, int sliceIdx, const SbBox2s &subSlice);
  void buildSubSliceY(void * output, int sliceIdx, const SbBox2s &subSlice);
  void buildSubSliceZ(void * output, int sliceIdx, const SbBox2s &subSlice);

private:
  SoVRMemReader * master;
};


#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

// *************************************************************************

SoVRMemReader::SoVRMemReader(void)
{
  PRIVATE(this) = new SoVRMemReaderP(this);

}

SoVRMemReader::~SoVRMemReader()
{
  delete PRIVATE(this);
}


void SoVRMemReader::setUserData(void * data)
{
}

void SoVRMemReader::getDataChar(SbBox3f &size, 
                                SoVolumeData::DataType &type, 
                                SbVec3s &dim)
{
  size = PRIVATE(this)->volumeSize;
  type = PRIVATE(this)->dataType;
  dim = PRIVATE(this)->dimensions;
}

void SoVRMemReader::getSubSlice(SbBox2s &subSlice, 
                                int sliceNumber, 
                                void * data, 
                                SoOrthoSlice::Axis axis)
{
  switch (axis) {
    case SoOrthoSlice::X:
      PRIVATE(this)->buildSubSliceX(data, sliceNumber, subSlice);
      break;

    case SoOrthoSlice::Y:
      PRIVATE(this)->buildSubSliceY(data, sliceNumber, subSlice);
      break;

    case SoOrthoSlice::Z:
      PRIVATE(this)->buildSubSliceZ(data, sliceNumber, subSlice);
      break;
  }
}


void 
SoVRMemReader::setData(const SbVec3s &dimensions, 
                       const void *data, 
                       const SbBox3f &volumeSize,
                       SoVolumeData::DataType type)
{
  PRIVATE(this)->dimensions = dimensions;
  PRIVATE(this)->data = data;
  PRIVATE(this)->dataType = type;
}



/*!
  Returns a raw image with Z as horisontal and Y as vertical axis
  Assumes that the provided data is in RGBA-form Caller deletes of
  course.

  This function and the similar functions for Y- and Z-axis should be
  fairly optimized. The innerloops could be unrolled a few times to
  get even more speed. But that would mess up the code.
*/
void 
SoVRMemReaderP::buildSubSliceX(void * output, 
                               int sliceIdx, 
                               const SbBox2s &subSlice)
{
  unsigned int * intData = (unsigned int *)data;
  unsigned int * intTexture = (unsigned int *)output;
  unsigned char * byteData = (unsigned char *)data;
  unsigned char * byteTexture = (unsigned char *)output;
  unsigned short * shortData = (unsigned short *)data;
  unsigned short * shortTexture = (unsigned short *)output;

  SbVec2s min, max;
  subSlice.getBounds(min, max);

  int out = 0;
  int xOffset = sliceIdx;
  int yOffset = min[1]*this->dimensions[0];
  int yLimit = max[1]*dimensions[0];
  int zAdd = dimensions[0]*dimensions[1];
  int zStart = min[0]*dimensions[0]*dimensions[1];

  while (yOffset < yLimit) {
    int zOffset = zStart + xOffset + yOffset;
    int zLimit  = max[0]*dimensions[0]*dimensions[1] 
                + xOffset + yOffset;

    switch (this->dataType) {

      case SoVolumeData::UNSIGNED_BYTE:
        while (zOffset < zLimit) {
          byteTexture[out] = byteData[zOffset];
          out ++;
          zOffset += zAdd;
        }
        break;

      case SoVolumeData::UNSIGNED_SHORT:
        while (zOffset < zLimit) {
          shortTexture[out] = shortData[zOffset];
          out ++;
          zOffset += zAdd;
        }
        break;

      case SoVolumeData::RGBA:
        while (zOffset < zLimit) {
          intTexture[out] = intData[zOffset];
          out ++;
          zOffset += zAdd;
        }
        break;
    }

    yOffset += dimensions[0];
  }
}




/*!
  Returns a texture with X as horisontal and Z as vertical axis
  Assumes that the provided data is in RGBA-form Caller deletes of
  course.
*/
void 
SoVRMemReaderP::buildSubSliceY(void * output, 
                               int sliceIdx, 
                               const SbBox2s &subSlice)
{
  unsigned int * intData = (unsigned int *)data;
  unsigned int * intTexture = (unsigned int *)output;
  unsigned char * byteData = (unsigned char *)data;
  unsigned char * byteTexture = (unsigned char *)output;
  unsigned short * shortData = (unsigned short *)data;
  unsigned short * shortTexture = (unsigned short *)output;

  SbVec2s min, max;
  subSlice.getBounds(min, max);

  int out = 0;
  int yOffset = sliceIdx*dimensions[0];
  int zOffset = min[1]*dimensions[0]*dimensions[1] + yOffset;
  int zLimit = dimensions[0]*dimensions[1]*max[1] + yOffset;

  while (zOffset < zLimit) {
    int xOffset = min[0] + zOffset;
    int xLimit = max[0] + zOffset;

    switch (this->dataType) {

      case SoVolumeData::UNSIGNED_BYTE:
          while (xOffset < xLimit) {
            byteTexture[out] = byteData[xOffset];
            out++;
            xOffset++;
          }
          break;

      case SoVolumeData::UNSIGNED_SHORT:
          while (xOffset < xLimit) {
            shortTexture[out] = shortData[xOffset];
            out++;
            xOffset++;
          }
          break;

      case SoVolumeData::RGBA:
          while (xOffset < xLimit) {
            intTexture[out] = intData[xOffset];
            out++;
            xOffset++;
          }
          break;

    }
    zOffset += dimensions[0]*dimensions[1];
  }
}


/*!
  Returns a texture with X as horisontal and Y as vertical axis
  Assumes that the provided data is in RGBA-form Caller deletes of
  course.
*/
void 
SoVRMemReaderP::buildSubSliceZ(void * output, 
                               int sliceIdx, 
                               const SbBox2s &subSlice)
{
  unsigned int * intData = (unsigned int *)data;
  unsigned int * intTexture = (unsigned int *)output;
  unsigned char * byteData = (unsigned char *)data;
  unsigned char * byteTexture = (unsigned char *)output;
  unsigned short * shortData = (unsigned short *)data;
  unsigned short * shortTexture = (unsigned short *)output;

  SbVec2s min, max;
  subSlice.getBounds(min, max);

  int out = 0;
  int zOffset = sliceIdx*dimensions[0]*dimensions[1];
  int yOffset = min[1]*dimensions[0] + zOffset;
  int yLimit = max[1]*dimensions[0] + zOffset;
  int xStart = min[0];
  while (yOffset < yLimit) {
    int xOffset = xStart + yOffset; 
    int xLimit = max[0] + yOffset; 

    switch (this->dataType) {
      case SoVolumeData::UNSIGNED_BYTE:
        while (xOffset < xLimit) {
          byteTexture[out] = byteData[xOffset];
          out++;
          xOffset++;
        }
        break;

      case SoVolumeData::UNSIGNED_SHORT:
        while (xOffset < xLimit) {
          shortTexture[out] = shortData[xOffset];
          out++;
          xOffset++;
        }
        break;

      case SoVolumeData::RGBA:
        while (xOffset < xLimit) {
          intTexture[out] = intData[xOffset];
          out++;
          xOffset++;
        }
        break;

    }
    // Next line of pixels
    yOffset += dimensions[0];
  }
}

void
SoVRMemReader::setVolumeSize(const SbBox3f &volumeSize)
{
  PRIVATE(this)->volumeSize = volumeSize;
}
