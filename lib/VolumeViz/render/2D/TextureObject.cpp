#include <VolumeViz/render/2D/CvrTextureObject.h>
#include <Inventor/C/tidbits.h>
#include <assert.h>

CvrTextureObject::CvrTextureObject(const SbVec2s & size)
{
  assert(coin_is_power_of_two(size[0]));
  assert(coin_is_power_of_two(size[1]));

  this->dimensions = size;
  this->rgbabuffer = NULL;
}

CvrTextureObject::~CvrTextureObject()
{
  delete[] this->rgbabuffer;
}

// Returns pointer to RGBA buffer. Allocates memory for it if
// necessary.
uint32_t *
CvrTextureObject::getRGBABuffer(void)
{
  if (this->rgbabuffer == NULL) {
    this->rgbabuffer = new uint32_t[this->dimensions[0] * this->dimensions[1]];
  }

  return this->rgbabuffer;
}

const SbVec2s &
CvrTextureObject::getDimensions(void) const
{
  return this->dimensions;
}
