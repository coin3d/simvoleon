// This datatype is a color lookup table.

#include <VolumeViz/misc/CvrCLUT.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <Inventor/SbBasic.h>
#include <Inventor/errors/SoDebugError.h>


// colormap values are between 0 and 255
CvrCLUT::CvrCLUT(const unsigned int nrcols, const uint8_t * colormap)
{
  this->nrentries = nrcols;
  this->nrcomponents = 4;
  this->datatype = INTS;

  const int blocksize = this->nrentries * this->nrcomponents;
  this->int_entries = new uint8_t[blocksize];
  (void)memcpy(this->int_entries, colormap, blocksize * sizeof(uint8_t));
  // (yeah, yeah, I know sizeof(uint8_t) is always == 1)

  this->commonConstructor();
}

// nrcomponents == 1: LUMINANCE
// nrcomponents == 2: LUMINANCE + ALPHA
// nrcomponents == 4: RGBA
//
// values in colormap are between 0.0 and 1.0
CvrCLUT::CvrCLUT(const unsigned int nrcols, const unsigned int nrcomponents,
                 const float * colormap)
{
  this->nrentries = nrcols;
  this->nrcomponents = nrcomponents;
  this->datatype = FLOATS;

  const int blocksize = this->nrentries * this->nrcomponents;
  this->flt_entries = new float[blocksize];
  (void)memcpy(this->int_entries, colormap, blocksize * sizeof(float));

  this->commonConstructor();
}

void
CvrCLUT::commonConstructor(void)
{
  this->refcount = 0;

  this->transparencythresholds[0] = 0;
  this->transparencythresholds[1] = this->nrentries - 1;

  this->alphapolicy = CvrCLUT::ALPHA_AS_IS;

  this->glcolors = new uint8_t[this->nrentries * 4];
  this->regenerateGLColorData();
}

CvrCLUT::~CvrCLUT()
{
  if (this->datatype == INTS)
    delete[] this->int_entries;
  if (this->datatype == FLOATS)
    delete[] this->flt_entries;

  delete[] this->glcolors;
}

void
CvrCLUT::ref(void) const
{
  CvrCLUT * that = (CvrCLUT *)this; // cast away constness
  that->refcount++;
}

void
CvrCLUT::unref(void) const
{
  CvrCLUT * that = (CvrCLUT *)this; // cast away constness
  that->refcount--;
  assert(this->refcount >= 0);
  if (this->refcount == 0) delete this;
}

int32_t
CvrCLUT::getRefCount(void) const
{
  return this->refcount;
}

// Everything below "low" (but not including) and above "high" (but
// not including), will be fully transparent.
void
CvrCLUT::setTransparencyThresholds(uint32_t low, uint32_t high)
{
  if ((this->transparencythresholds[0] == low) &&
      (this->transparencythresholds[1] == high)) {
    return;
  }

  assert(low <= high);
  assert(low < this->nrentries);
  assert(high < this->nrentries);

  this->transparencythresholds[0] = low;
  this->transparencythresholds[1] = high;

  this->regenerateGLColorData();
}

void
CvrCLUT::activate(const cc_glglue * glw) const
{
  // FIXME: should only need to do this once somewhere else
  glEnable(GL_COLOR_TABLE);

  // FIXME: handle this->nrentries != 256
  assert(this->nrentries == 256);

  // FIXME: should probably set glColorTableParameter() on
  // GL_COLOR_TABLE_SCALE and GL_COLOR_TABLE_BIAS.

  // FIXME: should probably check if all is ok by using
  // PROXY_TEXTURE_2D first.

  cc_glglue_glColorTable(glw,
                         GL_TEXTURE_2D, /* target */
                         GL_RGBA, /* GL internalformat */
                         this->nrentries, /* nr of paletteentries */
                         GL_RGBA, /* palette entry format */
                         GL_UNSIGNED_BYTE, /* palette entry unit type */
                         this->glcolors); /* data ptr */

  // Sanity check.
  assert(glGetError() == GL_NO_ERROR);

  // Sanity check.
  GLint actualsize;
  cc_glglue_glGetColorTableParameteriv(glw, GL_TEXTURE_2D,
                                       GL_COLOR_TABLE_WIDTH, &actualsize);
  assert(actualsize == (GLint)this->nrentries);
}

// Find RGBA color at the given idx.
void
CvrCLUT::lookupRGBA(const unsigned int idx, uint8_t rgba[4]) const
{
  assert(idx < this->nrentries);
  for (int i=0; i < 4; i++) { rgba[i] = this->glcolors[idx * 4 + i]; }
}

void
CvrCLUT::setAlphaUse(AlphaUse policy)
{
  if (this->alphapolicy == policy) { return; }

  this->alphapolicy = policy;
  this->regenerateGLColorData();
}

void
CvrCLUT::regenerateGLColorData(void)
{
  for (unsigned int idx = 0; idx < this->nrentries; idx++) {
    uint8_t * rgba = &this->glcolors[idx * 4];
    if ((idx < this->transparencythresholds[0]) ||
        (idx > this->transparencythresholds[1])) {
      rgba[0] = 0x00; rgba[1] = 0x00; rgba[2] = 0x00; rgba[3] = 0x00;
    }
    else {
      if (this->datatype == FLOATS) {
        const float * colvals = &(this->flt_entries[idx * this->nrcomponents]);
        switch (this->nrcomponents) {
        case 1: // ALPHA
          rgba[0] = rgba[1] = rgba[2] = rgba[3] = uint8_t(colvals[0] * 255.0f);
          break;

        case 2: // LUMINANCE_ALPHA
          rgba[0] = rgba[1] = rgba[2] = uint8_t(colvals[0] * 255.0f);
          rgba[3] = uint8_t(colvals[1] * 255.0f);
          break;

        case 4: // RGBA
          rgba[0] = uint8_t(colvals[0] * 255.0f);
          rgba[1] = uint8_t(colvals[1] * 255.0f);
          rgba[2] = uint8_t(colvals[2] * 255.0f);
          rgba[3] = uint8_t(colvals[3] * 255.0f);
          break;

        default:
          assert(FALSE && "impossible");
          break;
        }
      }
      else if (this->datatype == INTS) {
        const int colidx = idx * 4;
        rgba[0] = this->int_entries[colidx + 0];
        rgba[1] = this->int_entries[colidx + 1];
        rgba[2] = this->int_entries[colidx + 2];
        rgba[3] = this->int_entries[colidx + 3];
      }
      else assert(FALSE);
    }

    switch (this->alphapolicy) {
    case ALPHA_AS_IS: /* "as is", leave alone */ break;
    case ALPHA_OPAQUE: rgba[3] = 0xff; break;
    case ALPHA_BINARY: rgba[3] = (rgba[3] == 0) ? 0 : 0xff; break;
    default: assert(FALSE); break;
    }
  }
}
