#include <VolumeViz/render/common/CvrRGBATexture.h>
#include <Inventor/SbName.h>
#include <assert.h>

// Don't set value explicitly to SoType::badType(), to avoid a bug in
// Sun CC v4.0. (Bitpattern 0x0000 equals SoType::badType()).
SoType CvrRGBATexture::classTypeId;

SoType CvrRGBATexture::getTypeId(void) const { return CvrRGBATexture::classTypeId; }
SoType CvrRGBATexture::getClassTypeId(void) { return CvrRGBATexture::classTypeId; }

void
CvrRGBATexture::initClass(void)
{
  assert(CvrRGBATexture::classTypeId == SoType::badType());
  CvrRGBATexture::classTypeId =
    SoType::createType(CvrTextureObject::getClassTypeId(), "CvrRGBATexture");
}

CvrRGBATexture::CvrRGBATexture()
{
  assert(CvrRGBATexture::classTypeId != SoType::badType());
  this->rgbabuffer = NULL;
}

CvrRGBATexture::~CvrRGBATexture()
{
  if (this->rgbabuffer) delete[] this->rgbabuffer;
}

// Returns pointer to RGBA buffer. Allocates memory for it if
// necessary.
uint32_t *
CvrRGBATexture::getRGBABuffer(void) const
{ 
  return this->rgbabuffer;
}

