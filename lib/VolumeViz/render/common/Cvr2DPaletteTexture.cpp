#include <VolumeViz/render/common/Cvr2DPaletteTexture.h>
#include <VolumeViz/misc/CvrCLUT.h>
#include <Inventor/SbName.h>
#include <assert.h>

// Don't set value explicitly to SoType::badType(), to avoid a bug in
// Sun CC v4.0. (Bitpattern 0x0000 equals SoType::badType()).
SoType Cvr2DPaletteTexture::classTypeId;

SoType Cvr2DPaletteTexture::getTypeId(void) const { return Cvr2DPaletteTexture::classTypeId; }
SoType Cvr2DPaletteTexture::getClassTypeId(void) { return Cvr2DPaletteTexture::classTypeId; }

void
Cvr2DPaletteTexture::initClass(void)
{
  assert(Cvr2DPaletteTexture::classTypeId == SoType::badType());
  Cvr2DPaletteTexture::classTypeId =
    SoType::createType(CvrTextureObject::getClassTypeId(), "Cvr2DPaletteTexture");
}

Cvr2DPaletteTexture::Cvr2DPaletteTexture(const SbVec2s & size)
{
  assert(Cvr2DPaletteTexture::classTypeId != SoType::badType());
  this->dimensions = size;
}

Cvr2DPaletteTexture::~Cvr2DPaletteTexture()
{
}

// Returns pointer to buffer with 8-bit indices. Allocates memory for
// it if necessary.
uint8_t *
Cvr2DPaletteTexture::getIndex8Buffer(void) const
{
  if (this->indexbuffer == NULL) {
    // Cast away constness.
    Cvr2DPaletteTexture * that = (Cvr2DPaletteTexture *)this;
    that->indexbuffer = new uint8_t[this->dimensions[0] * this->dimensions[1]];
  }

  return this->indexbuffer;
}

// Blank out unused texture parts, to make sure we don't get any
// artifacts due to fp-inaccuracies when rendering.
void
Cvr2DPaletteTexture::blankUnused(const SbVec2s & texsize) const
{
  assert(this->indexbuffer);
  
  SbVec2s texobjdims = this->dimensions;
  {
    for (short y=texsize[1]; y < texobjdims[1]; y++) {
      for (short x=0; x < texobjdims[0]; x++) {
        this->indexbuffer[y * texobjdims[0] + x] = 0x00;
      }
    }
  }
  {
    for (short x=texsize[0]; x < texobjdims[0]; x++) {
      for (short y=0; y < texobjdims[1]; y++) {
        this->indexbuffer[y * texobjdims[0] + x] = 0x00;
      }
    }
  }

}


void
Cvr2DPaletteTexture::dumpToPPM(const char * filename) const
{
  FILE * f = fopen(filename, "w");
  assert(f);

  SbVec2s texobjdims = this->dimensions;
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
