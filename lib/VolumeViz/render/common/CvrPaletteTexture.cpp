#include <VolumeViz/render/common/CvrPaletteTexture.h>
#include <VolumeViz/misc/CvrCLUT.h>
#include <Inventor/SbName.h>
#include <assert.h>

// Don't set value explicitly to SoType::badType(), to avoid a bug in
// Sun CC v4.0. (Bitpattern 0x0000 equals SoType::badType()).
SoType CvrPaletteTexture::classTypeId;

SoType CvrPaletteTexture::getTypeId(void) const { return CvrPaletteTexture::classTypeId; }
SoType CvrPaletteTexture::getClassTypeId(void) { return CvrPaletteTexture::classTypeId; }

void
CvrPaletteTexture::initClass(void)
{
  assert(CvrPaletteTexture::classTypeId == SoType::badType());
  CvrPaletteTexture::classTypeId =
    SoType::createType(CvrTextureObject::getClassTypeId(), "CvrPaletteTexture");
}

CvrPaletteTexture::CvrPaletteTexture()
{
  assert(CvrPaletteTexture::classTypeId != SoType::badType());
  this->indexbuffer = NULL;
  this->clut = NULL;
}

CvrPaletteTexture::~CvrPaletteTexture()
{
  if (this->clut) this->clut->unref();
  if (this->indexbuffer) delete [] this->indexbuffer;
}

// Returns pointer to buffer with 8-bit indices.
uint8_t *
CvrPaletteTexture::getIndex8Buffer(void) const
{
  return this->indexbuffer;
}

void
CvrPaletteTexture::setCLUT(const CvrCLUT * table)
{
  if (this->clut) this->clut->unref();
  this->clut = table;
  this->clut->ref();
}

const CvrCLUT *
CvrPaletteTexture::getCLUT(void) const
{
  return this->clut;
}
