#include <VolumeViz/render/common/CvrTextureObject.h>
#include <VolumeViz/render/common/CvrRGBATexture.h>
#include <VolumeViz/render/common/CvrPaletteTexture.h>
#include <VolumeViz/render/common/Cvr2DRGBATexture.h>
#include <VolumeViz/render/common/Cvr2DPaletteTexture.h>
#include <VolumeViz/render/common/Cvr3DRGBATexture.h>
#include <VolumeViz/render/common/Cvr3DPaletteTexture.h>
#include <Inventor/C/tidbits.h>
#include <Inventor/SbName.h>
#include <assert.h>

// Don't set value explicitly to SoType::badType(), to avoid a bug in
// Sun CC v4.0. (Bitpattern 0x0000 equals SoType::badType()).
SoType CvrTextureObject::classTypeId;

SoType CvrTextureObject::getTypeId(void) const { return CvrTextureObject::classTypeId; }
SoType CvrTextureObject::getClassTypeId(void) { return CvrTextureObject::classTypeId; }


void
CvrTextureObject::initClass(void)
{
  assert(CvrTextureObject::classTypeId == SoType::badType());
  CvrTextureObject::classTypeId = SoType::createType(SoType::badType(),
                                                     "CvrTextureObject");

  CvrRGBATexture::initClass();
  CvrPaletteTexture::initClass();
  Cvr2DRGBATexture::initClass();
  Cvr2DPaletteTexture::initClass();
  Cvr3DRGBATexture::initClass();
  Cvr3DPaletteTexture::initClass();

}

CvrTextureObject::CvrTextureObject()
{
  assert(CvrTextureObject::classTypeId != SoType::badType());
}

CvrTextureObject::~CvrTextureObject()
{
}
