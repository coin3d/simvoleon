/*!
  \class SoVRMemReader VolumeViz/readers/SoVRMemReader.h
  \brief FIXME: doc
  \ingroup volviz
*/

#include <VolumeViz/readers/SoVRMemReader.h>

#include <Inventor/errors/SoDebugError.h>


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
                    const int pageidx, const SbBox2s & cutslice,
                    const SoVolumeData::DataType type, const SbVec3s & dim);
void buildSubSliceY(const void * input, void * output,
                    const int pageidx, const SbBox2s & cutslice,
                    const SoVolumeData::DataType type, const SbVec3s & dim);
void buildSubSliceZ(const void * input, void * output,
                    const int pageidx, const SbBox2s & cutslice,
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

void SoVRMemReader::getSubSlice(SbBox2s &subslice,
                                int sliceNumber,
                                void * data,
                                Axis axis)
{
  switch (axis) {
    case X:
      buildSubSliceX(this->m_data, data, sliceNumber, subslice,
                     PRIVATE(this)->dataType,
                     PRIVATE(this)->dimensions);
      break;

    case Y:
      buildSubSliceY(this->m_data, data, sliceNumber, subslice,
                     PRIVATE(this)->dataType,
                     PRIVATE(this)->dimensions);
      break;

    case Z:
      buildSubSliceZ(this->m_data, data, sliceNumber, subslice,
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


static unsigned int
datatype2bytesize(const SoVolumeData::DataType type)
{
  unsigned int voxelsize = 0;
  switch (type) {
  case SoVolumeData::UNSIGNED_BYTE: voxelsize = 1; break;
  case SoVolumeData::UNSIGNED_SHORT: voxelsize = 2; break;
  case SoVolumeData::RGBA: voxelsize = 4; break;
  default: assert(FALSE); break;
  }
  return voxelsize;
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
               const int pageidx,
               const SbBox2s & cutslice,
               const SoVolumeData::DataType type,
               const SbVec3s & dim)
{
  uint8_t * input8bits = (uint8_t *)input;
  uint8_t * output8bits = (uint8_t *)output;
  uint16_t * input16bits = (uint16_t *)input;
  uint16_t * output16bits = (uint16_t *)output;
  uint32_t * input32bits = (uint32_t *)input;
  uint32_t * output32bits = (uint32_t *)output;

  SbVec2s ssmin, ssmax;
  cutslice.getBounds(ssmin, ssmax);

  int xOffset = pageidx;
  int yOffset = ssmin[1] * dim[0];
  int yLimit = ssmax[1] * dim[0];
  int zAdd = dim[0] * dim[1];
  int zStart = ssmin[0] * dim[0] * dim[1];

  while (yOffset < yLimit) {
    int zOffset = zStart + xOffset + yOffset;
    int zLimit  = ssmax[0] * dim[0] * dim[1]
                + xOffset + yOffset;

    switch (type) {

      case SoVolumeData::UNSIGNED_BYTE:
        while (zOffset < zLimit) {
          *output8bits++ = input8bits[zOffset];
          zOffset += zAdd;
        }
        break;

      case SoVolumeData::UNSIGNED_SHORT:
        while (zOffset < zLimit) {
          *output16bits++ = input16bits[zOffset];
          zOffset += zAdd;
        }
        break;

      case SoVolumeData::RGBA:
        while (zOffset < zLimit) {
          *output32bits++ = input32bits[zOffset];
          zOffset += zAdd;
        }
        break;
    }

    yOffset += dim[0];
  }
}

// Copies rows of x-axis data along the z-axis.
void
buildSubSliceY(const void * input,
               void * output,
               const int pageidx,
               const SbBox2s & cutslice,
               const SoVolumeData::DataType type,
               const SbVec3s & dim)
{
  uint8_t * input8bits = (uint8_t *)input;
  uint8_t * output8bits = (uint8_t *)output;

  SbVec2s ssmin, ssmax;
  cutslice.getBounds(ssmin, ssmax);

  int yOffset = pageidx * dim[0];

  const unsigned int nrhorizvoxels = ssmax[0] - ssmin[0];
  assert(nrhorizvoxels > 0);
  const unsigned int nrvertvoxels = ssmax[1] - ssmin[1];
  assert(nrvertvoxels > 0);

  const unsigned int voxelsize = datatype2bytesize(type);

  for (unsigned int rowidx = 0; rowidx < nrvertvoxels; rowidx++) {
    const unsigned int inoffset =
      (yOffset + (ssmin[1] + rowidx) * dim[0] * dim[1] + ssmin[0]) * voxelsize;
    const uint8_t * srcptr = &(input8bits[inoffset]);

    // FIXME: nrhorizvoxels here should be actual width of
    // subpages, in case it's not 2^n. 20021125 mortene.
    uint8_t * dstptr = &(output8bits[nrhorizvoxels * rowidx * voxelsize]);

    (void)memcpy(dstptr, srcptr, nrhorizvoxels * voxelsize);
  }
}

// Copies rows of x-axis data along the y-axis.
void
buildSubSliceZ(const void * input, void * output,
               const int pageidx,
               const SbBox2s & cutslice,
               const SoVolumeData::DataType type,
               const SbVec3s & dim)
{
  uint8_t * input8bits = (uint8_t *)input;
  uint8_t * output8bits = (uint8_t *)output;

  assert(pageidx >= 0);
  assert(pageidx < dim[2]);

  SbVec2s ssmin, ssmax;
  cutslice.getBounds(ssmin, ssmax);
  const unsigned int nrhorizvoxels = ssmax[0] - ssmin[0];
  assert(nrhorizvoxels > 0);
  const unsigned int nrvertvoxels = ssmax[1] - ssmin[1];
  assert(nrvertvoxels > 0);

  const int zOffset = pageidx * dim[0] * dim[1];

  const unsigned int voxelsize = datatype2bytesize(type);

  for (unsigned int rowidx = 0; rowidx < nrvertvoxels; rowidx++) {
    const unsigned int inoffset = (zOffset + (ssmin[1] + rowidx) * dim[0] + ssmin[0]) * voxelsize;
    const uint8_t * srcptr = &(input8bits[inoffset]);

    // FIXME: nrhorizvoxels here should be actual width of
    // subpages, in case it's not 2^n. 20021125 mortene.
    uint8_t * dstptr = &(output8bits[nrhorizvoxels * rowidx * voxelsize]);

    (void)memcpy(dstptr, srcptr, nrhorizvoxels * voxelsize);
  }
}
