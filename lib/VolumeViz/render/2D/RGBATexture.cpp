#include <VolumeViz/render/2D/CvrRGBATexture.h>
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

CvrRGBATexture::CvrRGBATexture(const SbVec2s & size)
  : CvrTextureObject(size)
{
  assert(CvrRGBATexture::classTypeId != SoType::badType());

  this->rgbabuffer = NULL;
}

CvrRGBATexture::~CvrRGBATexture()
{
  delete[] this->rgbabuffer;
}

// Returns pointer to RGBA buffer. Allocates memory for it if
// necessary.
uint32_t *
CvrRGBATexture::getRGBABuffer(void) const
{
  if (this->rgbabuffer == NULL) {
    // Cast away constness.
    CvrRGBATexture * that = (CvrRGBATexture *)this;
    const SbVec2s & dim = this->getDimensions();
    that->rgbabuffer = new uint32_t[dim[0] * dim[1]];
  }

  return this->rgbabuffer;
}

// Blank out unused texture parts, to make sure we don't get any
// artifacts due to fp-inaccuracies when rendering.
void
CvrRGBATexture::blankUnused(const SbVec2s & texsize)
{
  assert(this->rgbabuffer);

  SbVec2s texobjdims = this->getDimensions();
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

void
CvrRGBATexture::dumpToPPM(const char * filename) const
{
  FILE * f = fopen(filename, "w");
  assert(f);

  SbVec2s texobjdims = this->getDimensions();
  // width height maxcolval
  (void)fprintf(f, "P3\n%d %d 255\n", texobjdims[0], texobjdims[1]);

  for (int i=0; i < texobjdims[0] * texobjdims[1]; i++) {
    uint32_t rgba = this->rgbabuffer[i];
    fprintf(f, "%d %d %d\n",
            rgba & 0xff, (rgba & 0xff00) >> 8,  (rgba & 0xff0000) >> 16);
  }
  fclose(f);
}
