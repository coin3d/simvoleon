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
#include <VolumeViz/misc/CvrUtil.h>
#include <VolumeViz/misc/CvrVoxelChunk.h>

#include <Inventor/C/tidbits.h>
#include <Inventor/errors/SoDebugError.h>

#include <errno.h>
#include <string.h>

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

// *************************************************************************

#define PRIVATE(p) (p->pimpl)
#define PUBLIC(p) (p->master)

class SoVRVolFileReaderP {
public:

  SoVRVolFileReaderP(void) {
    this->valid = FALSE;
  }

  static void debugDumpHeader(struct vol_header * vh);
  static SbBool debugFileRead(void);
  SoVolumeData::DataType dataType(void);

  struct vol_header volh;
  SbString description;
  SbBool valid;
};

/* Return value of CVR_DEBUG_IMPORT environment variable. */
SbBool
SoVRVolFileReaderP::debugFileRead(void)
{
  static int d = -1;
  if (d == -1) {
    const char * val = coin_getenv("CVR_DEBUG_IMPORT");
    d = val ? atoi(val) : 0;
  }
  return (d > 0) ? TRUE : FALSE;
}

SoVolumeData::DataType
SoVRVolFileReaderP::dataType(void)
{
  switch (this->volh.bits_per_voxel) {
  case 8: return SoVolumeData::UNSIGNED_BYTE;
  case 16: return SoVolumeData::UNSIGNED_SHORT;
  default: assert(FALSE && "unhandled voxel-size"); break;
  }
  return (SoVolumeData::DataType)0; // kills a compiler warning
}

void
SoVRVolFileReaderP::debugDumpHeader(struct vol_header * vh)
{
  SoDebugError::postInfo("SoVRVolFileReaderP::debugDumpHeader",
                         "magic_number==0x%08x, "
                         "header_length==%d (sizeof(struct vol_header)==%d), "
                         "width==%d, height==%d, images==%d, "
                         "bits_per_voxel==%d, index_bits==%d, "
                         "scaleX==%f, scaleY==%f, scaleZ==%f, "
                         "rotX==%f, rotY==%f, rotZ==%f",
                         vh->magic_number,
                         vh->header_length, sizeof(struct vol_header),
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
  assert(PRIVATE(this)->valid);

  type = PRIVATE(this)->dataType();

  struct vol_header * volh = &(PRIVATE(this)->volh);

  dim.setValue(volh->width, volh->height, volh->images);

  SbVec3f voldims(volh->width * volh->scaleX,
                  volh->height * volh->scaleY,
                  volh->images * volh->scaleZ);
  size.setBounds(-voldims/2.0f, voldims/2.0f);
}

void
SoVRVolFileReader::getSubSlice(SbBox2s & subslice, int slicenumber, void * data)
{
  assert(PRIVATE(this)->valid);

  struct vol_header * volh = &(PRIVATE(this)->volh);
  SbVec3s dims(volh->width, volh->height, volh->images);
  SoVolumeData::DataType type = PRIVATE(this)->dataType();

#if CVR_DEBUG && 0 // debug
  SbVec2s ssmin, ssmax;
  subslice.getBounds(ssmin, ssmax);
  SoDebugError::postInfo("SoVRVolFileReader::getSubSlice",
                         "slicenumber==%d"
                         "subslice==[%d, %d] [%d, %d]",
                         slicenumber,
                         ssmin[0], ssmin[1], ssmax[0], ssmax[1]);
#endif // debug

  CvrVoxelChunk::UnitSize vctype;
  switch (type) {
  case SoVolumeData::UNSIGNED_BYTE: vctype = CvrVoxelChunk::UINT_8; break;
  case SoVolumeData::UNSIGNED_SHORT: vctype = CvrVoxelChunk::UINT_16; break;
  case SoVolumeData::RGBA: vctype = CvrVoxelChunk::UINT_32; break;
  default: assert(FALSE); break;
  }

  // FIXME: interface of buildSubPage() should be improved to avoid
  // this roundabout way of clipping out a slice.  20021203 mortene.
  CvrVoxelChunk vc(dims, vctype, this->m_data);
  CvrVoxelChunk * output = vc.buildSubPage(2 /* Z */, slicenumber, subslice);
  (void)memcpy(data, output->getBuffer(), output->bufferSize());
  delete output;
}

void
SoVRVolFileReader::setUserData(void * data)
{
  const char * filename = (const char *)data;
  inherited::setFilename(filename);

  int64_t filesize = this->fileSize();
  if (filesize == -1) { return; }

  assert(filesize > 0);
  this->m_data = malloc(filesize);
  assert(this->m_data);

  FILE * f = fopen(filename, "rb");
  assert(f && "couldn't open file");
  // FIXME: move relevant code to
  // SoVolumeReader::getBuffer(). 20021125 mortene.
  size_t gotnrbytes = fread(this->m_data, 1, filesize, f);
  assert(gotnrbytes == filesize);
#if 1 // debug
  SoDebugError::postInfo("SoVRVolFileReader::setUserData",
                         "read %d bytes (%.2f MB)",
                         gotnrbytes, ((float)gotnrbytes) / 1024.0f / 1024.0f);
#endif // debug
  int r = fclose(f);
  assert(r == 0);

  assert(filesize > sizeof(struct vol_header));
  struct vol_header * volh = &PRIVATE(this)->volh;
  // magic_number and header_length
  (void)memcpy(volh, this->m_data, 2 * sizeof(uint32_t));
  volh->magic_number = coin_ntoh_uint32(volh->magic_number);
  volh->header_length = coin_ntoh_uint32(volh->header_length);

  // Set up sanitized defaults, in case the header is too short to
  // cover all fields -- which is actually allowed by the format.
  volh->width = coin_hton_uint32(0);
  volh->height = coin_hton_uint32(0);
  volh->images = coin_hton_uint32(0);
  volh->bits_per_voxel = coin_hton_uint32(8);
  volh->index_bits = coin_hton_uint32(0);
  // XXX
//   volh->scaleX = coin_hton_float(1.0f);
//   volh->scaleY = coin_hton_float(1.0f);
//   volh->scaleZ = coin_hton_float(1.0f);
  volh->scaleX = coin_hton_float(0.0f);
  volh->scaleY = coin_hton_float(0.0f);
  volh->scaleZ = coin_hton_float(0.0f);
  volh->rotX = coin_hton_float(0.0f);
  volh->rotY = coin_hton_float(0.0f);
  volh->rotZ = coin_hton_float(0.0f);

  const int copylen =
    SbMin((uint32_t)sizeof(struct vol_header), volh->header_length) - 2 * sizeof(uint32_t);

  (void)memcpy(&(volh->width),
               (uint8_t *)this->m_data + (2 * sizeof(uint32_t)),
               copylen);

  // FIXME: this actually fails with SYN_64.vol. 20021110 mortene.
//   assert(volh->header_length >= sizeof(struct vol_header));
  assert(volh->header_length < filesize);

  volh->width = coin_ntoh_uint32(volh->width);
  volh->height = coin_ntoh_uint32(volh->height);
  volh->images = coin_ntoh_uint32(volh->images);

  volh->bits_per_voxel = coin_ntoh_uint32(volh->bits_per_voxel);
  volh->index_bits = coin_ntoh_uint32(volh->index_bits);

  volh->scaleX = coin_ntoh_float(volh->scaleX);
  volh->scaleY = coin_ntoh_float(volh->scaleY);
  volh->scaleZ = coin_ntoh_float(volh->scaleZ);

  volh->rotX = coin_ntoh_float(volh->rotX);
  volh->rotY = coin_ntoh_float(volh->rotY);
  volh->rotZ = coin_ntoh_float(volh->rotZ);

  const char * descrptr = ((const char *)(this->m_data)) + sizeof(struct vol_header);
  PRIVATE(this)->description = descrptr;
  // FIXME: there's more descriptive text available after the first
  // '\0'. Must check header_length and convert '\0'-chars to
  // '\n'-chars. 20021109 mortene.

#if 0 // debug
  SoDebugError::postInfo("SoVRVolFileReader::setUserData",
                         "description=='%s'",
                         PRIVATE(this)->description.getString());
#endif // debug

  SoVRVolFileReaderP::debugDumpHeader(volh);

  // FIXME: this actually fails with LOBSTER.vol. 20021110 mortene.
//   assert(volh->magic_number == 0x0b7e7759);

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

  // Can't compare versus 0.0f directly, as that seems to fail for
  // several example VOL models I have.  --mortene.
  volh->scaleX = ((volh->scaleX < 0.0001f) ? 1.0f : volh->scaleX);
  volh->scaleY = ((volh->scaleY < 0.0001f) ? 1.0f : volh->scaleY);
  volh->scaleZ = ((volh->scaleZ < 0.0001f) ? 1.0f : volh->scaleZ);

  // FIXME: quick hack workaround for a problem with the SYN64 file:
  // the header stops right in the middle of scaleZ, and the
  // description overwrites scaleX and scaleY. 20021121 mortene.
  volh->scaleX = ((volh->scaleX > 1000000.0f) ? 1.0f : volh->scaleX);
  volh->scaleY = ((volh->scaleY > 1000000.0f) ? 1.0f : volh->scaleY);
  volh->scaleZ = ((volh->scaleZ > 1000000.0f) ? 1.0f : volh->scaleZ);

  uint32_t nrvoxels = volh->width * volh->height * volh->images;
  uint32_t minsize = (nrvoxels * volh->bits_per_voxel) / 8;
  assert(filesize >= minsize);

  // Shift actual voxel data to start at the m_data pointer.
  (void)memmove(this->m_data, (uint8_t *)this->m_data + volh->header_length,
                filesize - volh->header_length);

  PRIVATE(this)->valid = TRUE;
}

// *************************************************************************
