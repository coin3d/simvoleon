// This datatype is a color lookup table.

#include <VolumeViz/misc/CvrCLUT.h>
#include <assert.h>
#include <stdlib.h>
#include <Inventor/SbBasic.h>


CvrCLUT::CvrCLUT(const unsigned int nrcols, const uint8_t * rgba8bits)
{
  this->nrentries = nrcols;
  this->nrcomponents = 4;
  this->datatype = INTS;

  const int blocksize = this->nrentries * this->nrcomponents;
  this->int_entries = new uint8_t[blocksize];
  (void)memcpy(this->int_entries, rgba8bits, blocksize * sizeof(uint8_t));
  // (yeah, yeah, I know sizeof(uint8_t) is always == 1)

  this->commonConstructor();
}

// nrcomponents == 1: LUMINANCE
// nrcomponents == 2: LUMINANCE + ALPHA
// nrcomponents == 4: RGBA
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
  this->transparencythresholds[0] = 0;
  this->transparencythresholds[1] = this->nrentries - 1;

  // This is the block of fully transparent color values used by the
  // glColorSubTable() calls which implements the transparency
  // thresholds. It will be dynamically grown according to invocations
  // of setTransparencyThresholds().
  this->transparentblock = NULL;
  this->transparentblockentries = 0;
}

CvrCLUT::~CvrCLUT()
{
  if (this->datatype == INTS)
    delete[] this->int_entries;
  if (this->datatype == FLOATS)
    delete[] this->flt_entries;

  delete[] this->transparentblock;
}

// Everything below "low" (but not including) and above "high" (but
// not including), will be fully transparent.
void
CvrCLUT::setTransparencyThresholds(uint32_t low, uint32_t high)
{
  assert(low <= high);
  assert(low < this->nrentries);
  assert(high < this->nrentries);

  this->transparencythresholds[0] = low;
  this->transparencythresholds[1] = high;

  const int max_empty_entries = SbMax(low, this->nrentries - high - 1);
  if (this->transparentblockentries < max_empty_entries) {
    delete[] this->transparentblock;
    uint8_t * ptr = this->transparentblock =
      new uint8_t[max_empty_entries * 4];
    this->transparentblockentries = max_empty_entries;
    for (int i=0; i < this->transparentblockentries * 4; i++) { *ptr++ = 0x00; }
  } 
}

void
CvrCLUT::activate(const cc_glglue * glw) const
{
  // FIXME: should only need to do this once somewhere else
  glEnable(GL_COLOR_TABLE);

  // FIXME: handle this->nrentries != 256
  assert(this->nrentries == 256);

  void * dataptr = NULL;
  GLenum format, type;

  if (this->datatype == INTS) {
    dataptr = this->int_entries;
    type = GL_UNSIGNED_BYTE;
    format = GL_RGBA;
  }
  else if (this->datatype == FLOATS) {
    dataptr = this->flt_entries;
    type = GL_FLOAT;
    switch (this->nrcomponents) {
    case 1: format = GL_LUMINANCE; break;
    case 2: format = GL_LUMINANCE_ALPHA; break;
    case 4: format = GL_RGBA; break;
    default: assert(FALSE); break;
    }
  }
  else assert(FALSE);

  // FIXME: should probably set glColorTableParameter() on
  // GL_COLOR_TABLE_SCALE and GL_COLOR_TABLE_BIAS.

  // FIXME: should probably check if all is ok by using
  // PROXY_TEXTURE_2D first.

  cc_glglue_glColorTable(glw,
                         GL_TEXTURE_2D, /* target */
                         GL_RGBA, /* GL internalformat */
                         this->nrentries, /* nr of paletteentries */
                         format, /* palette entry format */
                         type, /* palette entry unit type */
                         dataptr); /* data ptr */

  // Sanity check.
  assert(glGetError() == GL_NO_ERROR);

  // Sanity check.
  GLint actualsize;
  cc_glglue_glGetColorTableParameteriv(glw, GL_TEXTURE_2D,
                                       GL_COLOR_TABLE_WIDTH, &actualsize);
  assert(actualsize == (GLint)this->nrentries);


  // Set up transparent areas of the colortable.

  uint32_t low = this->transparencythresholds[0];
  uint32_t high = this->transparencythresholds[1];
  uint32_t numtrailing = this->nrentries - high - 1;

  if (low > 0) {
    cc_glglue_glColorSubTable(glw,
                              GL_TEXTURE_2D, /* target */
                              0, /* start */
                              low, /* count */
                              GL_RGBA, /* palette entry format */
                              GL_UNSIGNED_BYTE, /* palette entry unit type */
                              this->transparentblock); /* data ptr */
  }

  if (high < (this->nrentries - 1)) {
    cc_glglue_glColorSubTable(glw,
                              GL_TEXTURE_2D, /* target */
                              high + 1, /* start */
                              numtrailing, /* count */
                              GL_RGBA, /* palette entry format */
                              GL_UNSIGNED_BYTE, /* palette entry unit type */
                              this->transparentblock); /* data ptr */
  }

  assert(glGetError() == GL_NO_ERROR);
}
