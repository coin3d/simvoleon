/**************************************************************************\
 * Copyright (c) Kongsberg Oil & Gas Technologies AS
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
\**************************************************************************/

/*!
  \class SoVRVolFileReader VolumeViz/readers/SoVRVolFileReader.h
  \brief Loader for files in the VOL data format.

  This class reads volume data from files in the data format used in
  the book «Introduction To Volume Rendering», by Lichtenbelt, Crane
  and Naqvi (Hewlett-Packard / Prentice Hall), ISBN 0-13-861683-3.

  The format is just a simple header, before the voxel values are laid
  out in raw, uncompressed form:

  \verbatim
  struct vol_header {
    uint32_t magic_number;
    uint32_t header_length;
    uint32_t width;
    uint32_t height;
    uint32_t images;
    uint32_t bits_per_voxel;
    uint32_t index_bits;
    float scaleX, scaleY, scaleZ;
    float rotX, rotY, rotZ;
  };
  \endverbatim

  Volume data will by default be \e normalized to be within a 2x2x2
  unit dimensions cube. As an example, if you set up a voxel data set
  of dimensions 100x400x200, this will be rendered within a bounding
  box of <-0.25, -1, -0.5> to <0.25, 1, 0.5>. Notice that the largest
  dimension (the Y dimension in this example) will be made to fit
  within unit size 2, and the other dimensions will be scaled
  accordingly.

  The scale vector (if present in the header) will subsequently be
  used on the resulting normalized volume dimensions.

  You may use SoVolumeData::setVolumeSize() to force a different unit
  size box around the volume, or you can simply use the standard Coin
  transformation nodes, like e.g. SoScale, to accomplish this.

  For more information about how the data will be mapped to the world
  coordinate system, see the documentation of
  SoVolumeData::setVolumeData().
*/

// *************************************************************************

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
  // FIXME: messy, use arrays. 20041007 mortene.
  float scaleX, scaleY, scaleZ;
  float rotX, rotY, rotZ;
};

// workaround for buggy coin_hton_float()/coin_ntoh_float() (obsoleted
// in Coin-4) 

// FIXME: consider just using coin_hton_float_bytes() and
// coin_ntoh_float_bytes() instead. They were added quite recently
// though, which means SimVoleon won't compile for older versions of
// Coin

namespace {
  void hton_float(float v, float * dst) {
    union {
      float f32;
      uint32_t u32;
    } val;
    val.f32 = v;
    uint32_t * dsti = reinterpret_cast<uint32_t*> (dst);
    *dsti = coin_hton_uint32(val.u32);
  }
  float ntoh_float(const float * src) {
    const uint32_t * srci = reinterpret_cast<const uint32_t*> (src);
    union {
      float f32;
      uint32_t u32;
    } val;
    val.u32 = *srci;
    val.u32 = coin_ntoh_uint32(val.u32);
    return val.f32;
  }
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

// Documented in superclass.
void
SoVRVolFileReader::getDataChar(SbBox3f & size, SoVolumeData::DataType & type,
                               SbVec3s & dim)
{
  assert(PRIVATE(this)->valid);

  type = PRIVATE(this)->dataType();

  struct vol_header * volh = &(PRIVATE(this)->volh);

  dim.setValue(volh->width, volh->height, volh->images);

  const short largestdimension = SbMax(dim[0], SbMax(dim[1], dim[2]));
  SbVec3f normdims(dim[0], dim[1], dim[2]);
  normdims /= float(largestdimension);
  normdims *= 2.0f;

  SbVec3f scale(volh->scaleX, volh->scaleY, volh->scaleZ);

  for (unsigned int i=0; i < 3; i++) { normdims[i] *= scale[i]; }
  size.setBounds(-normdims / 2.0f, normdims / 2.0f);
}

// Documented in superclass.
//
// FIXME: this is supposed to be the sole interface (well, together
// with the new getSubCube() (or whatever its name is) function TGS
// has added in later VolumeViz versions) for getting at the volume
// data. The point of this is that one can implement optimized import
// if the end-user has a roaming interface (with the SoROI node)
// against the data, showing only smaller parts of it at the same
// time. We should really support this properly, as it can be very
// useful when working with huge volumes. 20040629 mortene.
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

  unsigned int bytesprvoxel;
  switch (type) {
  case SoVolumeData::UNSIGNED_BYTE: bytesprvoxel = 1; break;
  case SoVolumeData::UNSIGNED_SHORT: bytesprvoxel = 2; break;
  default: assert(FALSE); break;
  }

  // FIXME: interface of buildSubPage() should be improved to avoid
  // this roundabout way of clipping out a slice.  20021203 mortene.
  CvrVoxelChunk vc(dims, bytesprvoxel, this->m_data);
  CvrVoxelChunk * output = vc.buildSubPage(2 /* Z */, slicenumber, subslice);
  (void)memcpy(data, output->getBuffer(), output->bufferSize());
  delete output;
}

/*!
  \a data should be a pointer to a character string with the full
  filename of a file in VOL-format.
*/
void
SoVRVolFileReader::setUserData(void * data)
{
  const char * filename = (const char *)data;
  inherited::setFilename(filename);

  int64_t filesize = this->fileSize();
  if (filesize == -1) { return; }

  assert(filesize > 0);
  // FIXME: this is completely bogus use of SoVolumeReader::m_data --
  // this is *not* where the voxel data is supposed to be stored. That
  // is inside SoVolumeData. 20041008 mortene.
  this->m_data = malloc((size_t)filesize); // FIXME: bogus cast. 2004121 mortene.
  assert(this->m_data);

  FILE * f = fopen(filename, "rb");
  assert(f && "couldn't open file");
  // FIXME: move relevant code to
  // SoVolumeReader::getBuffer(). 20021125 mortene.
  size_t gotnrbytes =
    fread(this->m_data, 1, (size_t)filesize, f); // FIXME: bogus cast. 2004121 mortene.
  assert(gotnrbytes == (size_t)filesize); // FIXME: bogus cast. 2004121 mortene.

  if (CvrUtil::doDebugging()) {
    SoDebugError::postInfo("SoVRVolFileReader::setUserData",
                           "read %d bytes (%.2f MB)",
                           gotnrbytes, ((float)gotnrbytes) / 1024.0f / 1024.0f);
  }

  int r = fclose(f);
  assert(r == 0);

  assert((uint64_t)filesize > sizeof(struct vol_header));
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

  hton_float(0.0f, &volh->scaleX);
  hton_float(0.0f, &volh->scaleY);
  hton_float(0.0f, &volh->scaleZ);
  hton_float(0.0f, &volh->rotX);
  hton_float(0.0f, &volh->rotY);
  hton_float(0.0f, &volh->rotZ);

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

  volh->scaleX = ntoh_float(&volh->scaleX);
  volh->scaleY = ntoh_float(&volh->scaleY);
  volh->scaleZ = ntoh_float(&volh->scaleZ);

  volh->rotX = ntoh_float(&volh->rotX);
  volh->rotY = ntoh_float(&volh->rotY);
  volh->rotZ = ntoh_float(&volh->rotZ);

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

  if (CvrUtil::doDebugging()) { SoVRVolFileReaderP::debugDumpHeader(volh); }

  // FIXME: this actually fails with LOBSTER.vol. 20021110 mortene.
  // assert(volh->magic_number == 0x0b7e7759);

  assert((volh->width > 0) && (volh->width < 32767));
  assert((volh->height > 0) && (volh->height < 32767));
  assert((volh->images > 0) && (volh->images < 32767));

  assert(volh->bits_per_voxel >= 1);

  // FIXME: bad, but tmp, limitation. Fails with one of the "Intro 2
  // VR" models. 20021110 mortene.
  assert((volh->bits_per_voxel - volh->index_bits) == 8);

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
                ((size_t)filesize) - volh->header_length); // FIXME: bogus cast. 2004121 mortene.

  const char * env = coin_getenv("CVR_DEBUG_DUMP_RAW");
  if (env) {
    FILE * f = fopen(env, "w");
    assert(f); // FIXME: handle in robust manner. 20030702 mortene.
    // FIXME: error check next two. 20030702 mortene.
    fwrite(this->m_data, 1,
           ((size_t)filesize) - volh->header_length, f); // FIXME: bogus cast. 2004121 mortene.
    fclose(f);
  }

  PRIVATE(this)->valid = TRUE;
}

// *************************************************************************
