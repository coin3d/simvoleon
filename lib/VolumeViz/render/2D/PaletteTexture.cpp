#include <VolumeViz/render/2D/CvrPaletteTexture.h>
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

CvrPaletteTexture::CvrPaletteTexture(const SbVec2s & size)
  : CvrTextureObject(size)
{
  assert(CvrPaletteTexture::classTypeId != SoType::badType());

  this->indexbuffer = NULL;
  this->clut = NULL;
}

CvrPaletteTexture::~CvrPaletteTexture()
{
  delete[] this->indexbuffer;
}

// Returns pointer to buffer with 8-bit indices. Allocates memory for
// it if necessary.
uint8_t *
CvrPaletteTexture::getIndex8Buffer(void) const
{
  if (this->indexbuffer == NULL) {
    // Cast away constness.
    CvrPaletteTexture * that = (CvrPaletteTexture *)this;
    const SbVec2s & dim = this->getDimensions();
    that->indexbuffer = new uint8_t[dim[0] * dim[1]];
  }

  return this->indexbuffer;
}

void
CvrPaletteTexture::setCLUT(const CvrCLUT * table)
{
  this->clut = table;
  // FIXME: do reference counting here on the CvrCLUT
}

const CvrCLUT *
CvrPaletteTexture::getCLUT(void) const
{
  return this->clut;
}

void
CvrPaletteTexture::dumpToPPM(const char * filename) const
{
  FILE * f = fopen(filename, "w");
  assert(f);

  SbVec2s texobjdims = this->getDimensions();
  // width height maxcolval
  (void)fprintf(f, "P3\n%d %d 255\n", texobjdims[0], texobjdims[1]);

  for (int i=0; i < texobjdims[0] * texobjdims[1]; i++) {
#if 0
    uint8_t index = this->indexbuffer[i];
    // FIXME: convert index value to rgba
    fprintf(f, "%d %d %d\n",
            rgba & 0xff, (rgba & 0xff00) >> 8,  (rgba & 0xff0000) >> 16);
#else
    assert(FALSE && "yet unimplemented");
#endif
  }
  fclose(f);
}
