/*!
  \class SoVRVolFileReader VolumeViz/readers/SoVRVolFileReader.h
  \brief Loader for files in the VOL data format.
  \ingroup volviz

  This class reads volume data from files in the data format used in
  the book «Introduction To Volume Rendering», by Lichtenbelt, Crane
  and Naqvi (Hewlett-Packard / Prentice Hall), ISBN 0-13-861683-3.

  The data format is laid out as follows:

  First there is a header block like this:

  FIXME: fill in this space.. 20021108 mortene.
*/


#include <VolumeViz/readers/SoVRVolFileReader.h>
#include <Inventor/errors/SoDebugError.h>

struct vol_header {
  uint32_t magic_number;
  uint32_t header_length;
  uint32_t width;
  uint32_t height;
  uint32_t images;
  uint32_t bits_per_voxel;
  uint32_t index_bits;
  // FIXME: should assert-chk that sizeof(float)==4. 20021109 mortene.
  float scaleX, scaleY, scaleZ;
  float rotX, rotY, rotZ;
};

// FIXME: expose tidbits.h properly from Coin. 20021109 mortene.
extern "C" {
extern uint32_t coin_hton_uint32(uint32_t value);
extern uint32_t coin_ntoh_uint32(uint32_t value);
}

// FIXME: refactor properly. 20021110 mortene.
extern void buildSubSliceX(const void * input, void * output,
                           int sliceIdx, const SbBox2s & subSlice,
                           const SoVolumeData::DataType type, const SbVec3s & dim);
extern void buildSubSliceY(const void * input, void * output,
                           int sliceIdx, const SbBox2s & subSlice,
                           const SoVolumeData::DataType type, const SbVec3s & dim);
extern void buildSubSliceZ(const void * input, void * output,
                           int sliceIdx, const SbBox2s & subSlice,
                           const SoVolumeData::DataType type, const SbVec3s & dim);

// *************************************************************************

#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

class SoVRVolFileReaderP {
public:

  SoVRVolFileReaderP(void) {
    this->dataType = SoVolumeData::UNSIGNED_BYTE;
    this->dimensions = SbVec3s(0, 0, 0);
  }

  SbVec3s dimensions;
  SoVolumeData::DataType dataType;

  static void debugDumpHeader(struct vol_header * vh);

  struct vol_header volh;
  SbString description;
};

void
SoVRVolFileReaderP::debugDumpHeader(struct vol_header * vh)
{
  SoDebugError::postInfo("SoVRVolFileReaderP::debugDumpHeader",
                         "magic_number==0x%08x, "
                         "header_length==%d, "
                         "width==%d, height==%d, images==%d, "
                         "bits_per_voxel==%d, index_bits==%d, "
                         "scaleX==%f, scaleY==%f, scaleZ==%f, "
                         "rotX==%f, rotY==%f, rotZ==%f",
                         vh->magic_number,
                         vh->header_length,
                         vh->width, vh->height, vh->images,
                         vh->bits_per_voxel, vh->index_bits,
                         vh->scaleX, vh->scaleY, vh->scaleZ,
                         vh->rotX, vh->rotY, vh->rotZ);
}

// *************************************************************************


SoVRVolFileReader::SoVRVolFileReader(void)
{
  PRIVATE(this) = new SoVRVolFileReaderP;
}

SoVRVolFileReader::~SoVRVolFileReader()
{
  delete PRIVATE(this);
}

void
SoVRVolFileReader::getDataChar(SbBox3f & size, SoVolumeData::DataType & type,
                               SbVec3s & dim)
{
  type = PRIVATE(this)->dataType;
  dim = PRIVATE(this)->dimensions;

  struct vol_header * volh = &(PRIVATE(this)->volh);
  SbVec3f voldims(volh->width * volh->scaleX,
                  volh->height * volh->scaleY,
                  volh->images * volh->scaleZ);
  size.setBounds(-voldims/2.0f, voldims/2.0f);
}

void
SoVRVolFileReader::getSubSlice(SbBox2s & subslice, int slicenumber, void * data,
                               Axis axis)
{
  switch (axis) {
    case X:
      buildSubSliceX(this->m_data, data, slicenumber, subslice,
                     PRIVATE(this)->dataType,
                     PRIVATE(this)->dimensions);
      break;

    case Y:
      buildSubSliceY(this->m_data, data, slicenumber, subslice,
                     PRIVATE(this)->dataType,
                     PRIVATE(this)->dimensions);
      break;

    case Z:
      buildSubSliceZ(this->m_data, data, slicenumber, subslice,
                     PRIVATE(this)->dataType,
                     PRIVATE(this)->dimensions);
      break;
  }
}

void
SoVRVolFileReader::setUserData(void * data)
{
  const char * filename = (const char *)data;
  inherited::setFilename(filename);

  int64_t filesize = this->fileSize();
  assert(filesize > 0);
  this->m_data = malloc(filesize);
  assert(this->m_data);

  FILE * f = fopen(filename, "rb");
  assert(f && "couldn't open file");
  size_t gotnrbytes = fread(this->m_data, 1, filesize, f);
  assert(gotnrbytes == filesize);
#if 1 // debug
  SoDebugError::postInfo("SoVRVolFileReader::setUserData",
                         "read %d bytes", gotnrbytes);
#endif // debug
  int r = fclose(f);
  assert(r == 0);

#if 1 // debug
  SoDebugError::postInfo("SoVRVolFileReader::setUserData",
                         "sizeof(struct vol_header)==%d",
                         sizeof(struct vol_header));
#endif // debug

  assert(filesize > sizeof(struct vol_header));
  struct vol_header * volh = &PRIVATE(this)->volh;
  (void)memcpy(volh, this->m_data, sizeof(struct vol_header));

  volh->magic_number = coin_ntoh_uint32(volh->magic_number);

  volh->header_length = coin_ntoh_uint32(volh->header_length);

  volh->width = coin_ntoh_uint32(volh->width);
  volh->height = coin_ntoh_uint32(volh->height);
  volh->images = coin_ntoh_uint32(volh->images);

  volh->bits_per_voxel = coin_ntoh_uint32(volh->bits_per_voxel);
  volh->index_bits = coin_ntoh_uint32(volh->index_bits);

  // FIXME: ugly casting. 20021109 mortene.
  volh->scaleX = (float)coin_ntoh_uint32((uint32_t)volh->scaleX);
  volh->scaleY = (float)coin_ntoh_uint32((uint32_t)volh->scaleY);
  volh->scaleZ = (float)coin_ntoh_uint32((uint32_t)volh->scaleZ);

  volh->rotX = (float)coin_ntoh_uint32((uint32_t)volh->rotX);
  volh->rotY = (float)coin_ntoh_uint32((uint32_t)volh->rotY);
  volh->rotZ = (float)coin_ntoh_uint32((uint32_t)volh->rotZ);

  const char * descrptr = ((const char *)(this->m_data)) + sizeof(struct vol_header);
  PRIVATE(this)->description = descrptr;
  // FIXME: there's more descriptive text available after the first
  // '\0'. Must check header_length and convert '\0'-chars to
  // '\n'-chars. 20021109 mortene.

#if 1 // debug
  SoDebugError::postInfo("SoVRVolFileReader::setUserData",
                         "description=='%s'",
                         PRIVATE(this)->description.getString());
#endif // debug

  SoVRVolFileReaderP::debugDumpHeader(volh);

  // FIXME: this actually fails with LOBSTER.vol. 20021110 mortene.
//   assert(volh->magic_number == 0x0b7e7759);

  // FIXME: this actually fails with SYN_64.vol. 20021110 mortene.
//   assert((volh->header_length >= sizeof(struct vol_header)) &&
//          (volh->header_length < filesize));

  assert((volh->width > 0) && (volh->width < 32767));
  assert((volh->height > 0) && (volh->height < 32767));
  assert((volh->images > 0) && (volh->images < 32767));

  assert(volh->bits_per_voxel >= 1);
  // FIXME: bad, but tmp, limitation. Fails with one of the "Intro 2
  // VR" models. 20021110 mortene.
  assert(volh->bits_per_voxel == 8);

  assert(volh->scaleX >= 0.0f);
  assert(volh->scaleY >= 0.0f);
  assert(volh->scaleZ >= 0.0f);

  // FIXME: this should hopefully not be necessary when proper header
  // read is in place. 20021120 mortene.
  volh->scaleX = ((volh->scaleX == 0.0f) ? 1.0f : volh->scaleX);
  volh->scaleY = ((volh->scaleY == 0.0f) ? 1.0f : volh->scaleY);
  volh->scaleZ = ((volh->scaleZ == 0.0f) ? 1.0f : volh->scaleZ);

  uint32_t nrvoxels = volh->width * volh->height * volh->images;
  uint32_t minsize = (nrvoxels * volh->bits_per_voxel) / 8;
  assert(filesize >= minsize);


  PRIVATE(this)->dimensions = SbVec3s(volh->width, volh->height, volh->images);
}

// *************************************************************************
