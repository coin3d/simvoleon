#include <VolumeViz/render/2D/CvrTextureObject.h>
#include <VolumeViz/render/2D/CvrRGBATexture.h>
#include <VolumeViz/render/2D/CvrPaletteTexture.h>
#include <Inventor/C/tidbits.h>
#include <Inventor/SbName.h>
#include <assert.h>

// Don't set value explicitly to SoType::badType(), to avoid a bug in
// Sun CC v4.0. (Bitpattern 0x0000 equals SoType::badType()).
SoType CvrTextureObject::classTypeId;

void
CvrTextureObject::initClass(void)
{
  assert(CvrTextureObject::classTypeId == SoType::badType());
  CvrTextureObject::classTypeId = SoType::createType(SoType::badType(),
                                                     "CvrTextureObject");

  CvrRGBATexture::initClass();
  CvrPaletteTexture::initClass();
}

CvrTextureObject::CvrTextureObject(const SbVec2s & size)
{
  assert(CvrTextureObject::classTypeId != SoType::badType());

  assert(coin_is_power_of_two(size[0]));
  assert(coin_is_power_of_two(size[1]));
  this->dimensions = size;
}

CvrTextureObject::~CvrTextureObject()
{
}

const SbVec2s &
CvrTextureObject::getDimensions(void) const
{
  return this->dimensions;
}

SoType
CvrTextureObject::getClassTypeId(void)
{
  return CvrTextureObject::classTypeId;
}
