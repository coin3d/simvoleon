/*!
  \class SoVRMemReader VolumeViz/readers/SoVRMemReader.h
  \brief FIXME: doc
  \ingroup volviz
*/

#include <VolumeViz/readers/SoVRMemReader.h>


class SoVRMemReaderP {
public:
  SoVRMemReaderP(SoVRMemReader * master) {
    this->master = master;

    this->dimensions = SbVec3s(0, 0, 0);
    this->dataType = SoVolumeData::UNSIGNED_BYTE;
  }

  SbVec3s dimensions;
  SoVolumeData::DataType dataType;

private:
  SoVRMemReader * master;
};

#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

void buildSubSliceX(const void * input, void * output,
                    int sliceIdx, const SbBox2s & subSlice,
                    const SoVolumeData::DataType type, const SbVec3s & dim);
void buildSubSliceY(const void * input, void * output,
                    int sliceIdx, const SbBox2s & subSlice,
                    const SoVolumeData::DataType type, const SbVec3s & dim);
void buildSubSliceZ(const void * input, void * output,
                    int sliceIdx, const SbBox2s & subSlice,
                    const SoVolumeData::DataType type, const SbVec3s & dim);


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

void SoVRMemReader::getDataChar(SbBox3f & size,
                                SoVolumeData::DataType & type,
                                SbVec3s & dim)
{
  type = PRIVATE(this)->dataType;
  dim = PRIVATE(this)->dimensions;

  size.setBounds(-dim[0]/2.0f, -dim[1]/2.0f, -dim[2]/2.0f,
                 dim[0]/2.0f, dim[1]/2.0f, dim[2]/2.0f);
}

void SoVRMemReader::getSubSlice(SbBox2s &subSlice,
                                int sliceNumber,
                                void * data,
                                Axis axis)
{
  switch (axis) {
    case X:
      buildSubSliceX(this->m_data, data, sliceNumber, subSlice,
                     PRIVATE(this)->dataType,
                     PRIVATE(this)->dimensions);
      break;

    case Y:
      buildSubSliceY(this->m_data, data, sliceNumber, subSlice,
                     PRIVATE(this)->dataType,
                     PRIVATE(this)->dimensions);
      break;

    case Z:
      buildSubSliceZ(this->m_data, data, sliceNumber, subSlice,
                     PRIVATE(this)->dataType,
                     PRIVATE(this)->dimensions);
      break;
  }
}


void
SoVRMemReader::setData(const SbVec3s &dimensions,
                       void * data,
                       SoVolumeData::DataType type)
{
  PRIVATE(this)->dimensions = dimensions;
  this->m_data = data;
  PRIVATE(this)->dataType = type;
}



/*!
  Returns a raw image with Z as horisontal and Y as vertical axis.
  Assumes that the provided data is in RGBA-form. Caller deletes of
  course.

  This function and the similar functions for Y- and Z-axis should be
  fairly optimized. The innerloops could be unrolled a few times to
  get even more speed. But that would mess up the code.
*/
void
buildSubSliceX(const void * input,
               void * output,
               int sliceIdx,
               const SbBox2s & subSlice,
               const SoVolumeData::DataType type,
               const SbVec3s & dim)
{
  // FIXME: use fixed-width int-types. 20021109 mortene.
  unsigned int * intData = (unsigned int *)input;
  unsigned int * intTexture = (unsigned int *)output;
  unsigned char * byteData = (unsigned char *)input;
  unsigned char * byteTexture = (unsigned char *)output;
  unsigned short * shortData = (unsigned short *)input;
  unsigned short * shortTexture = (unsigned short *)output;

  SbVec2s min, max;
  subSlice.getBounds(min, max);

  int out = 0;
  int xOffset = sliceIdx;
  int yOffset = min[1]*dim[0];
  int yLimit = max[1]*dim[0];
  int zAdd = dim[0]*dim[1];
  int zStart = min[0]*dim[0]*dim[1];

  while (yOffset < yLimit) {
    int zOffset = zStart + xOffset + yOffset;
    int zLimit  = max[0]*dim[0]*dim[1]
                + xOffset + yOffset;

    switch (type) {

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

    yOffset += dim[0];
  }
}




/*!
  Returns a texture with X as horisontal and Z as vertical axis
  Assumes that the provided data is in RGBA-form Caller deletes of
  course.
*/
void
buildSubSliceY(const void * input,
               void * output,
               int sliceIdx,
               const SbBox2s & subSlice,
               const SoVolumeData::DataType type,
               const SbVec3s & dim)
{
  unsigned int * intData = (unsigned int *)input;
  unsigned int * intTexture = (unsigned int *)output;
  unsigned char * byteData = (unsigned char *)input;
  unsigned char * byteTexture = (unsigned char *)output;
  unsigned short * shortData = (unsigned short *)input;
  unsigned short * shortTexture = (unsigned short *)output;

  SbVec2s min, max;
  subSlice.getBounds(min, max);

  int out = 0;
  int yOffset = sliceIdx*dim[0];
  int zOffset = min[1]*dim[0]*dim[1] + yOffset;
  int zLimit = dim[0]*dim[1]*max[1] + yOffset;

  while (zOffset < zLimit) {
    int xOffset = min[0] + zOffset;
    int xLimit = max[0] + zOffset;

    switch (type) {

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
    zOffset += dim[0]*dim[1];
  }
}


/*!
  Returns a texture with X as horisontal and Y as vertical axis
  Assumes that the provided data is in RGBA-form Caller deletes of
  course.
*/
void
buildSubSliceZ(const void * input,
               void * output,
               int sliceIdx,
               const SbBox2s & subSlice,
               const SoVolumeData::DataType type,
               const SbVec3s & dim)
{
  unsigned int * intData = (unsigned int *)input;
  unsigned int * intTexture = (unsigned int *)output;
  unsigned char * byteData = (unsigned char *)input;
  unsigned char * byteTexture = (unsigned char *)output;
  unsigned short * shortData = (unsigned short *)input;
  unsigned short * shortTexture = (unsigned short *)output;

  SbVec2s min, max;
  subSlice.getBounds(min, max);

  int out = 0;
  int zOffset = sliceIdx*dim[0]*dim[1];
  int yOffset = min[1]*dim[0] + zOffset;
  int yLimit = max[1]*dim[0] + zOffset;
  int xStart = min[0];
  while (yOffset < yLimit) {
    int xOffset = xStart + yOffset;
    int xLimit = max[0] + yOffset;

    switch (type) {
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
    yOffset += dim[0];
  }
}
