#include <VolumeViz/misc/CvrUtil.h>
#include <VolumeViz/misc/CvrVoxelChunk.h>
#include <string.h>
#include <Inventor/C/tidbits.h>

// Returns a value indicating whether or not to spit out debugging
// information during execution. Debugging is turned on by the user by
// setting the CVR_DEBUG environment variable to "1".
SbBool
CvrUtil::doDebugging(void)
{
  static int do_debugging = -1;
  if (do_debugging == -1) {
    const char * envstr = coin_getenv("CVR_DEBUG");
    do_debugging = envstr && atoi(envstr) > 0;
  }
  return do_debugging == 1 ? TRUE : FALSE;
}


// Cut a slice along any principal axis, of a single image depth.
CvrVoxelChunk *
CvrUtil::buildSubPage(const CvrVoxelChunk & input,
                      const unsigned int axisidx, const int pageidx,
                      const SbBox2s & cutslice)
{
  CvrVoxelChunk * output = NULL;
  switch (axisidx) {
  case 0:
    output = CvrUtil::buildSubPageX(input, pageidx, cutslice);
    break;

  case 1:
    output = CvrUtil::buildSubPageY(input, pageidx, cutslice);
    break;

  case 2:
    output = CvrUtil::buildSubPageZ(input, pageidx, cutslice);
    break;

  default:
    assert(FALSE);
    break;
  }
  return output;
}


// Copies rows of z-axis data down the y-axis.
CvrVoxelChunk *
CvrUtil::buildSubPageX(const CvrVoxelChunk & input,
                       const int pageidx, // FIXME: get rid of this by using an SbBox3s for cutslice. 20021203 mortene.
                       const SbBox2s & cutslice)
{
  assert(pageidx >= 0);
  assert(pageidx < input.getDimensions()[0]);

  SbVec2s ssmin, ssmax;
  cutslice.getBounds(ssmin, ssmax);

  const SbVec3s dim = input.getDimensions();

  const int zAdd = dim[0] * dim[1];

  const unsigned int nrhorizvoxels = ssmax[0] - ssmin[0];
  assert(nrhorizvoxels > 0);
  const unsigned int nrvertvoxels = ssmax[1] - ssmin[1];
  assert(nrvertvoxels > 0);

  const SbVec3s outputdims(nrhorizvoxels, nrvertvoxels, 1);
  CvrVoxelChunk * output = new CvrVoxelChunk(outputdims, input.getUnitSize());

  const unsigned int staticoffset =
    pageidx + ssmin[1] * dim[0] + ssmin[0] * zAdd;

  const unsigned int voxelsize = input.getUnitSize();
  uint8_t * inputbytebuffer = (uint8_t *)input.getBuffer();
  uint8_t * outputbytebuffer = (uint8_t *)output->getBuffer();

  for (unsigned int rowidx = 0; rowidx < nrvertvoxels; rowidx++) {
    const unsigned int inoffset = staticoffset + (rowidx * dim[0]);
    const uint8_t * srcptr = &(inputbytebuffer[inoffset * voxelsize]);
    uint8_t * dstptr = &(outputbytebuffer[nrhorizvoxels * rowidx * voxelsize]);

    // FIXME: should optimize this loop. 20021125 mortene.
    for (unsigned int horizidx = 0; horizidx < nrhorizvoxels; horizidx++) {
      *dstptr++ = *srcptr++;
      if (voxelsize > 1) *dstptr++ = *srcptr++;
      if (voxelsize == 4) { *dstptr++ = *srcptr++; *dstptr++ = *srcptr++; }

      srcptr += zAdd * voxelsize - voxelsize;
    }
  }

  return output;
}

// Copies rows of x-axis data along the z-axis.
/*
  Here's how a 4x3 slice would be cut, from the memory layout:

  +----------------+
  |                |
  |    xxxx        |
  |                |
  |                |
  |                |
  +----------------+
  |                |
  |    xxxx        |
  |                |
  |                |
  |                |
  +----------------+
  |                |
  |    xxxx        |
  |                |
  |                |
  |                |
  +----------------+

  Cutting is done top-to-bottom, and is placed in the output slice
  buffer left-to-right, top-to-bottom.
 */
CvrVoxelChunk *
CvrUtil::buildSubPageY(const CvrVoxelChunk & input,
                       const int pageidx, // FIXME: get rid of this by using an SbBox3s for cutslice. 20021203 mortene.
                       const SbBox2s & cutslice)
{
  assert(pageidx >= 0);
  assert(pageidx < input.getDimensions()[1]);

  SbVec2s ssmin, ssmax;
  cutslice.getBounds(ssmin, ssmax);

  const SbVec3s dim = input.getDimensions();

  const unsigned int nrhorizvoxels = ssmax[0] - ssmin[0];
  assert(nrhorizvoxels > 0);
  const unsigned int nrvertvoxels = ssmax[1] - ssmin[1];
  assert(nrvertvoxels > 0);

  const SbVec3s outputdims(nrhorizvoxels, nrvertvoxels, 1);
  CvrVoxelChunk * output = new CvrVoxelChunk(outputdims, input.getUnitSize());

  const unsigned int staticoffset =
    (ssmin[1] * dim[0] * dim[1]) + (pageidx * dim[0]) + ssmin[0];

  const unsigned int voxelsize = input.getUnitSize();
  uint8_t * inputbytebuffer = (uint8_t *)input.getBuffer();
  uint8_t * outputbytebuffer = (uint8_t *)output->getBuffer();

  for (unsigned int rowidx = 0; rowidx < nrvertvoxels; rowidx++) {
    const unsigned int inoffset = staticoffset + (rowidx * dim[0] * dim[1]);
    const uint8_t * srcptr = &(inputbytebuffer[inoffset * voxelsize]);

    uint8_t * dstptr = &(outputbytebuffer[nrhorizvoxels * rowidx * voxelsize]);

    (void)memcpy(dstptr, srcptr, nrhorizvoxels * voxelsize);
  }

  return output;
}

// Copies rows of x-axis data down the y-axis.
CvrVoxelChunk *
CvrUtil::buildSubPageZ(const CvrVoxelChunk & input,
                       const int pageidx, // FIXME: get rid of this by using an SbBox3s for cutslice. 20021203 mortene.
                       const SbBox2s & cutslice)
{
  assert(pageidx >= 0);
  assert(pageidx < input.getDimensions()[2]);

  SbVec2s ssmin, ssmax;
  cutslice.getBounds(ssmin, ssmax);

  const SbVec3s dim = input.getDimensions();

  const unsigned int nrhorizvoxels = ssmax[0] - ssmin[0];
  assert(nrhorizvoxels > 0);
  const unsigned int nrvertvoxels = ssmax[1] - ssmin[1];
  assert(nrvertvoxels > 0);

  const SbVec3s outputdims(nrhorizvoxels, nrvertvoxels, 1);
  CvrVoxelChunk * output = new CvrVoxelChunk(outputdims, input.getUnitSize());

  const unsigned int staticoffset =
    (pageidx * dim[0] * dim[1]) + (ssmin[1] * dim[0]) + ssmin[0];

  const unsigned int voxelsize = input.getUnitSize();
  uint8_t * inputbytebuffer = (uint8_t *)input.getBuffer();
  uint8_t * outputbytebuffer = (uint8_t *)output->getBuffer();

  for (unsigned int rowidx = 0; rowidx < nrvertvoxels; rowidx++) {
    const unsigned int inoffset = staticoffset + (rowidx * dim[0]);
    const uint8_t * srcptr = &(inputbytebuffer[inoffset * voxelsize]);

    uint8_t * dstptr = &(outputbytebuffer[nrhorizvoxels * rowidx * voxelsize]);

    (void)memcpy(dstptr, srcptr, nrhorizvoxels * voxelsize);
  }

  return output;
}
