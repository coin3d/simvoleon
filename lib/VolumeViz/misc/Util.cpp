#include <VolumeViz/misc/CvrUtil.h>

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

// Copies rows of z-axis data down the y-axis.
void
CvrUtil::buildSubSliceX(const void * input,
                        void * output,
                        const int pageidx,
                        const SbBox2s & cutslice,
                        const unsigned short destwidth,
                        const SoVolumeData::DataType type,
                        const SbVec3s & dim)
{
  uint8_t * input8bits = (uint8_t *)input;
  uint8_t * output8bits = (uint8_t *)output;

  assert(pageidx >= 0);
  assert(pageidx < dim[1]);

  SbVec2s ssmin, ssmax;
  cutslice.getBounds(ssmin, ssmax);

  int zAdd = dim[0] * dim[1];

  const unsigned int nrhorizvoxels = ssmax[0] - ssmin[0];
  assert(nrhorizvoxels > 0);
  const unsigned int nrvertvoxels = ssmax[1] - ssmin[1];
  assert(nrvertvoxels > 0);

  const unsigned int staticoffset =
    pageidx + ssmin[1] * dim[0] + ssmin[0] * zAdd;

  const unsigned int voxelsize = datatype2bytesize(type);

  for (unsigned int rowidx = 0; rowidx < nrvertvoxels; rowidx++) {
    const unsigned int inoffset = staticoffset + (rowidx * dim[0]);
    const uint8_t * srcptr = &(input8bits[inoffset * voxelsize]);

    // We're using destwidth instead of nrhorizvoxels here in case the
    // actual width of subpages is different from the cutslice
    // size. This can happen out towards the borders of the
    // volumedata-set if volumedatadimension % subpagesize != 0.
    uint8_t * dstptr = &(output8bits[destwidth * rowidx * voxelsize]);

    // FIXME: try to optimize this loop. 20021125 mortene.
    for (unsigned int horizidx = 0; horizidx < nrhorizvoxels; horizidx++) {
      *dstptr++ = *srcptr++;
      if (voxelsize > 1) *dstptr++ = *srcptr++;
      if (voxelsize == 4) { *dstptr++ = *srcptr++; *dstptr++ = *srcptr++; }

      srcptr += zAdd * voxelsize - voxelsize;
    }
  }
}

// Copies rows of x-axis data along the z-axis.
void
CvrUtil::buildSubSliceY(const void * input,
                        void * output,
                        const int pageidx,
                        const SbBox2s & cutslice,
                        const unsigned short destwidth,
                        const SoVolumeData::DataType type,
                        const SbVec3s & dim)
{
  uint8_t * input8bits = (uint8_t *)input;
  uint8_t * output8bits = (uint8_t *)output;

  assert(pageidx >= 0);
  assert(pageidx < dim[1]);

  SbVec2s ssmin, ssmax;
  cutslice.getBounds(ssmin, ssmax);

  const unsigned int nrhorizvoxels = ssmax[0] - ssmin[0];
  assert(nrhorizvoxels > 0);
  const unsigned int nrvertvoxels = ssmax[1] - ssmin[1];
  assert(nrvertvoxels > 0);

  const unsigned int staticoffset =
    (ssmin[1] * dim[0] * dim[1]) + (pageidx * dim[0]) + ssmin[0];

  const unsigned int voxelsize = datatype2bytesize(type);

  for (unsigned int rowidx = 0; rowidx < nrvertvoxels; rowidx++) {
    const unsigned int inoffset = staticoffset + (rowidx * dim[0] * dim[1]);
    const uint8_t * srcptr = &(input8bits[inoffset * voxelsize]);

    // We're using destwidth instead of nrhorizvoxels here in case the
    // actual width of subpages is different from the cutslice
    // size. This can happen out towards the borders of the
    // volumedata-set if volumedatadimension % subpagesize != 0.
    uint8_t * dstptr = &(output8bits[destwidth * rowidx * voxelsize]);

    (void)memcpy(dstptr, srcptr, nrhorizvoxels * voxelsize);
  }
}

// Copies rows of x-axis data down the y-axis.
void
CvrUtil::buildSubSliceZ(const void * input, void * output,
                        const int pageidx,
                        const SbBox2s & cutslice,
                        const unsigned short destwidth,
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

  const unsigned int staticoffset =
    (pageidx * dim[0] * dim[1]) + (ssmin[1] * dim[0]) + ssmin[0];

  const unsigned int voxelsize = datatype2bytesize(type);

  for (unsigned int rowidx = 0; rowidx < nrvertvoxels; rowidx++) {
    const unsigned int inoffset = staticoffset + (rowidx * dim[0]);
    const uint8_t * srcptr = &(input8bits[inoffset * voxelsize]);

    // We're using destwidth instead of nrhorizvoxels here in case the
    // actual width of subpages is different from the cutslice
    // size. This can happen out towards the borders of the
    // volumedata-set if volumedatadimension % subpagesize != 0.
    uint8_t * dstptr = &(output8bits[destwidth * rowidx * voxelsize]);

    (void)memcpy(dstptr, srcptr, nrhorizvoxels * voxelsize);
  }
}
