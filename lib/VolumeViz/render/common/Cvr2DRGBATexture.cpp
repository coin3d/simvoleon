#include <VolumeViz/render/common/Cvr2DRGBATexture.h>
#include <Inventor/SbName.h>
#include <assert.h>

// Don't set value explicitly to SoType::badType(), to avoid a bug in
// Sun CC v4.0. (Bitpattern 0x0000 equals SoType::badType()).
SoType Cvr2DRGBATexture::classTypeId;

SoType Cvr2DRGBATexture::getTypeId(void) const { return Cvr2DRGBATexture::classTypeId; }
SoType Cvr2DRGBATexture::getClassTypeId(void) { return Cvr2DRGBATexture::classTypeId; }

void
Cvr2DRGBATexture::initClass(void)
{
  assert(Cvr2DRGBATexture::classTypeId == SoType::badType());
  Cvr2DRGBATexture::classTypeId =
    SoType::createType(CvrTextureObject::getClassTypeId(), "Cvr2DRGBATexture");
}

Cvr2DRGBATexture::Cvr2DRGBATexture(const SbVec2s & size)
{
  assert(Cvr2DRGBATexture::classTypeId != SoType::badType());
  this->dimensions = size;
}

Cvr2DRGBATexture::~Cvr2DRGBATexture()
{
}

// Returns pointer to RGBA buffer. Allocates memory for it if
// necessary.
uint32_t *
Cvr2DRGBATexture::getRGBABuffer(void) const
{
  if (this->rgbabuffer == NULL) {
    // Cast away constness.
    Cvr2DRGBATexture * that = (Cvr2DRGBATexture *)this;
    that->rgbabuffer = new uint32_t[this->dimensions[0] * this->dimensions[1]];
  }

  return this->rgbabuffer;
}

// Blank out unused texture parts, to make sure we don't get any
// artifacts due to fp-inaccuracies when rendering.
void
Cvr2DRGBATexture::blankUnused(const SbVec2s & texsize) const
{
  assert(this->rgbabuffer);

  SbVec2s texobjdims = this->dimensions;
  {
    for (short y=texsize[1]; y < texobjdims[1]; y++) {
      for (short x=0; x < texobjdims[0]; x++) {
        this->rgbabuffer[y * texobjdims[0] + x] = 0x00000000;
      }
    }
  }
  {
    for (short x=texsize[0]; x < texobjdims[0]; x++) {
      for (short y=0; y < texobjdims[1]; y++) {
        this->rgbabuffer[y * texobjdims[0] + x] = 0x00000000;
      }
    }
  }
}

