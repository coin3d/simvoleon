#include <VolumeViz/misc/CvrVoxelChunk.h>
#include <assert.h>

// Allocates an uninitialized buffer for storing enough voxel data to
// fit into the given dimensions with space per voxel allocated
// according to the UnitSize type.
//
// If the "buffer" argument is non-NULL, will nto allocate a buffer,
// but rather just use that pointer. It is then the caller's
// responsibility to a) not destruct that buffer before this instance
// is destructed, and b) to deallocate the buffer data.
CvrVoxelChunk::CvrVoxelChunk(const SbVec3s & dimensions, UnitSize type,
                             void * buffer)
{
  assert(dimensions[0] > 0);
  assert(dimensions[1] > 0);
  assert(dimensions[2] > 0);
  assert(type == UINT_8 || type == UINT_16 || type == UINT_32);

  this->dimensions = dimensions;
  this->unitsize = type;

  if (buffer == NULL) {
    this->voxelbuffer = new uint8_t[this->bufferSize()];
    this->destructbuffer = TRUE;
  }
  else {
    this->voxelbuffer = buffer;
    this->destructbuffer = FALSE;
  }
}

CvrVoxelChunk::~CvrVoxelChunk()
{
  if (this->destructbuffer) { delete[] (uint8_t *)this->voxelbuffer; }
}

// Number of bytes in buffer.
unsigned int
CvrVoxelChunk::bufferSize(void) const
{
  return
    this->dimensions[0] * this->dimensions[1] * this->dimensions[2] *
    this->unitsize;
}

// Returns the buffer start pointer.
void *
CvrVoxelChunk::getBuffer(void) const
{
  return this->voxelbuffer;
}

// Returns the buffer start pointer. Convenience method to return the
// pointer casted to the correct size. Don't use this method unless
// the unitsize type is UINT_8.
uint8_t *
CvrVoxelChunk::getBuffer8(void) const
{
  assert(this->unitsize == UINT_8);
  return (uint8_t *)this->voxelbuffer;
}

// Returns the buffer start pointer. Convenience method to return the
// pointer casted to the correct size. Don't use this method unless
// the unitsize type is UINT_16.
uint16_t *
CvrVoxelChunk::getBuffer16(void) const
{
  assert(this->unitsize == UINT_16);
  return (uint16_t *)this->voxelbuffer;
}

// Returns the buffer start pointer. Convenience method to return the
// pointer casted to the correct size. Don't use this method unless
// the unitsize type is UINT_32.
uint32_t *
CvrVoxelChunk::getBuffer32(void) const
{
  assert(this->unitsize == UINT_32);
  return (uint32_t *)this->voxelbuffer;
}

const SbVec3s &
CvrVoxelChunk::getDimensions(void) const
{
  return this->dimensions;
}

CvrVoxelChunk::UnitSize
CvrVoxelChunk::getUnitSize(void) const
{
  return this->unitsize;
}
