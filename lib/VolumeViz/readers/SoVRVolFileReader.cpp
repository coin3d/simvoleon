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

// *************************************************************************

class SoVRVolFileReaderP {
public:
  static void debugDumpHeader(struct vol_header * vh);
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
}

SoVRVolFileReader::~SoVRVolFileReader()
{
}

void
SoVRVolFileReader::getDataChar(SbBox3f & size, SoVolumeData::DataType & type,
                               SbVec3s & dim)
{
}

void
SoVRVolFileReader::getSubSlice(SbBox2s & subslice, int slicenumber, void * data,
                               Axis axis)
{
}

void
SoVRVolFileReader::setUserData(void * data)
{
  const char * filename = (const char *)data;
  inherited::setFilename(filename);

  int64_t filesize = this->fileSize();
  this->m_data = malloc(filesize);

  FILE * f = fopen(filename, "rb");
  assert(f && "couldn't open file");
  size_t gotnrbytes = fread(this->m_data, 1, filesize, f);
#if 1 // debug
  SoDebugError::postInfo("SoVRVolFileReader::setUserData",
                         "read %d bytes", gotnrbytes);
#endif // debug
  (void)fclose(f);

  struct vol_header volh;
  (void)memcpy(&volh, this->m_data, sizeof(struct vol_header));

  volh.magic_number = coin_ntoh_uint32(volh.magic_number);

  volh.header_length = coin_ntoh_uint32(volh.header_length);
  volh.width = coin_ntoh_uint32(volh.width);
  volh.height = coin_ntoh_uint32(volh.height);
  volh.images = coin_ntoh_uint32(volh.images);
  volh.bits_per_voxel = coin_ntoh_uint32(volh.bits_per_voxel);
  volh.index_bits = coin_ntoh_uint32(volh.index_bits);
  // FIXME: ugly casting. 20021109 mortene.
  volh.scaleX = (float)coin_ntoh_uint32((uint32_t)volh.scaleX);
  volh.scaleY = (float)coin_ntoh_uint32((uint32_t)volh.scaleY);
  volh.scaleZ = (float)coin_ntoh_uint32((uint32_t)volh.scaleZ);
  volh.rotX = (float)coin_ntoh_uint32((uint32_t)volh.rotX);
  volh.rotY = (float)coin_ntoh_uint32((uint32_t)volh.rotY);
  volh.rotZ = (float)coin_ntoh_uint32((uint32_t)volh.rotZ);

  const char * descrptr = ((const char *)(this->m_data)) + sizeof(struct vol_header);
  SbString description = descrptr;
  // FIXME: there's more descriptive text available after the first
  // '\0'. Must check header_length and convert '\0'-chars to
  // '\n'-chars. 20021109 mortene.

#if 1 // debug
  SoDebugError::postInfo("SoVRVolFileReader::setUserData",
                         "description=='%s'", description.getString());
#endif // debug

  SoVRVolFileReaderP::debugDumpHeader(&volh);
}

// *************************************************************************
